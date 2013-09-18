/*
 *  pix_videoDarwin.cpp
 *  gem_darwin
 *
 *  Created by James Tittle on Fri Jul 12 2002.
 *  Copyright (c) 2002-2005 James Tittle & Chris Clepper
 *
 */
#include "Gem/GemConfig.h"


#if defined __APPLE__ && !defined __x86_64__
// with OSX10.6, apple has removed loads of Carbon functionality (in 64bit mode)
// LATER make this a real check in configure
# define HAVE_CARBONQUICKTIME
#endif

#if !defined HAVE_CARBONQUICKTIME  && defined GEM_VIDEOBACKEND && GEM_VIDEOBACKEND == GEM_VIDEOBACKEND_Darwin
# undef GEM_VIDEOBACKEND
#endif

#if defined GEM_VIDEOBACKEND && GEM_VIDEOBACKEND == GEM_VIDEOBACKEND_Darwin

#define HELPSYMBOL "pix_video"

#include "pix_videoDarwin.h"
#include "Gem/Cache.h"
#include "Gem/State.h"

#include <Carbon/Carbon.h>

#include <unistd.h> //needed for Unix file open() type functions
#include <stdio.h>
#include <fcntl.h> /* JMZ thinks that _this_ is needed for open() */

CPPEXTERN_NEW_WITH_TWO_ARGS(pix_videoDarwin, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

#define DEFAULT_WIDTH        320
#define DEFAULT_HEIGHT        240
#define DEFAULT_FRAMES        5        // 5 fps
#define MAX_RECORDING_TIME    100 * 60    // n * 60 ticks  (n : seconds)
#define DEFAULT_INTERVAL    5        // 5 milliseconds for calling SGIdle()

pix_videoDarwin :: pix_videoDarwin( t_floatarg w, t_floatarg h ) :
    m_srcGWorld(NULL)
{

  if (w > 0 ){
    m_vidXSize = (int)w;
  }else{
    m_vidXSize = 320;
  }
  if (h > 0){
    m_vidYSize = (int)h;
  }else{
    m_vidYSize = 240;
  }

  m_haveVideo = 0;
  logpost(NULL, 3, "height %d width %d",m_vidXSize,m_vidYSize);
  m_pixBlock.image.xsize = m_vidXSize;
  m_pixBlock.image.ysize = m_vidYSize;

  m_pixBlock.image.csize = 4;
  m_pixBlock.image.format = GL_BGRA_EXT;
  /* shouldn't this be GL_UNSIGNED_INT_8_8_8_8 on macintel?
   * LATER use setCsizeByFormat() instead
   */
  m_pixBlock.image.type = GL_UNSIGNED_INT_8_8_8_8_REV;

  m_pixBlock.image.allocate();

  m_quality = 0; //normal quality gives non-interlaced images from DV cams
  m_colorspace = GL_YCBCR_422_GEM; //default to YUV

  //set to the first input device
  m_inputDevice = 0;
  m_auto = 1; //keeps previous default functionailty
  m_banged = false;
  InitSeqGrabber();
  m_record = 0;
  //startTransfer();

}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_videoDarwin :: ~pix_videoDarwin()
{
   if (m_vc) {
        if (::SGDisposeChannel(m_sg, m_vc)) {
            error ("Unable to dispose a video channel");
        }
        m_vc = NULL;
    }
    if (m_sg) {
        if (::CloseComponent(m_sg)) {
            error("Unable to dispose a sequence grabber component");
        }
        m_sg = NULL;
        if (m_srcGWorld) {
        ::DisposeGWorld(m_srcGWorld);
        m_pixMap = NULL;
        m_srcGWorld = NULL;
        m_baseAddr = NULL;
    }
    }
}
/////////////////////////////////////////////////////////
// startrender
//
/////////////////////////////////////////////////////////
void pix_videoDarwin :: startRendering()
{
     m_haveVideo = 1;
     m_pixBlock.newimage = 1;
}

/////////////////////////////////////////////////////////
// startrender
//
/////////////////////////////////////////////////////////
void pix_videoDarwin :: stopRendering()
{
    //this should stop the recording process
    m_record = 0;
    setupCapture();
    //stopTransfer();
}
/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_videoDarwin :: render(GemState *state)
{
    OSErr    err;
//    short    frameCount = 0;
//    Boolean    *done;

    if (m_auto || m_banged) {

    err = SGIdle(m_sg);

    if (err != noErr){
            error("SGIdle failed with error %d",err);
            m_haveVideo = 0;
        } else {
        //this doesn't do anything so far
        //VDCompressDone(m_vdig,frameCount,data,size,similar,time);
        //err = SGGrabFrameComplete(m_vc,frameCount,done);
        //if (err != noErr) error("SGGrabCompressComplete failed with error %d",err);
        //post("SGGrabFramecomplete done %d framecount = %d",done[0],frameCount);

        m_haveVideo = 1;
        m_newFrame = 1;
        }
    if (!m_haveVideo)
    {
        post("no video yet");
        return;
    }
//    derSwizzler(m_pixBlock.image);
    m_pixBlock.newimage = m_newFrame;
    state->image = &m_pixBlock;
    m_newFrame = 0;

  }else {

    m_newFrame = 0;
    m_pixBlock.newimage = m_newFrame;
    state->image = &m_pixBlock;

  }

  m_banged = false;
}

/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void pix_videoDarwin :: postrender(GemState *state)
{
    m_pixBlock.newimage = 0;
    state->image = NULL;

}

/////////////////////////////////////////////////////////
// startTransfer
//
/////////////////////////////////////////////////////////
int pix_videoDarwin :: startTransfer()
{
    OSErr    err = noErr;

    if (m_record)    {
        SGStartRecord(m_sg);
        if (err != noErr)error("SGStartRecord failed with error %d",err);
        }
    else SGStartPreview(m_sg);
     m_haveVideo = 1;
     m_pixBlock.newimage = 1;
     return 1;
}

/////////////////////////////////////////////////////////
// startTransfer
//
/////////////////////////////////////////////////////////
int pix_videoDarwin :: stopTransfer()
{
    OSErr    err = noErr;

    //might need SGPause or SGStop here
    err = SGStop(m_sg);
    if (err != noErr)error("SGStop failed with error %d",err);
     return 1;
}

void pix_videoDarwin :: DoVideoSettings()
{
    Rect    newActiveVideoRect;
    Rect    curBounds, curVideoRect, newVideoRect;
    ComponentResult    err;


    // Get our current state - do i need this???
    err = SGGetChannelBounds (m_vc, &curBounds);
    err = SGGetVideoRect (m_vc, &curVideoRect);

    // Pause
    err = SGPause (m_sg, true);

    // Do the dialog thang

    err = SGSettingsDialog( m_sg, m_vc, 0, nil, 0, nil, 0);


    // What happened?
    err = SGGetVideoRect (m_vc, &newVideoRect);
    err = SGGetSrcVideoBounds (m_vc, &newActiveVideoRect);

    /* //Attempt to save SG settings to disk
    UserData    uD = NULL;
    //NewUserData(uD);
    SGGetSettings(m_sg,&uD,0);
    //short uDCount;
    //uDCount = CountUserDataType(*uD,sgClipType);
    //post("UserDataType count %d",uDCount);

    Handle myHandle = NewHandle(0);
    int length;

    PutUserDataIntoHandle(uD,myHandle);
    length = GetHandleSize(myHandle);

    int myFile;
    myFile = open("/Users/lincoln/Documents/temp",O_CREAT | O_RDWR, 0600);
    write(myFile,myHandle,length);
    close(myFile);
    */

    err = SGPause (m_sg, false);
}


void pix_videoDarwin :: InitSeqGrabber()
{
    OSErr anErr;
    Rect m_srcRect = {0,0, m_vidYSize, m_vidXSize};

    SGDeviceList    devices;
    short            deviceIndex,inputIndex;
    short            deviceCount = 0;
    SGDeviceInputList theSGInputList = NULL;
    bool showInputsAsDevices;
//    UserData     *uD;


    /*
    int num_components = 0;
    Component c = 0;
    ComponentDescription cd;

     cd.componentType = SeqGrabComponentType;
     cd.componentSubType = 0;
     cd.componentManufacturer = 0;
     cd.componentFlags = 0;
     cd.componentFlagsMask = 0;

     while((c = FindNextComponent(c, &cd)) != 0) {
       num_components++;  }                 // add component c to the list.
  //   post("number of SGcomponents: %d",num_components);
  */
    m_sg = OpenDefaultComponent(SeqGrabComponentType, 0);
    if(m_sg==NULL){
        error("could not open default component");
        return;
    }
    anErr = SGInitialize(m_sg);
    if(anErr!=noErr){
        error("could not initialize SG error %d",anErr);
        return;
    }

    anErr = SGSetDataRef(m_sg, 0, 0, seqGrabDontMakeMovie);
        if (anErr != noErr){
            error("dataref failed with error %d",anErr);
        }

    anErr = SGNewChannel(m_sg, VideoMediaType, &m_vc);
    if(anErr!=noErr){
        error("could not make new SG channnel error %d",anErr);
        return;
    }

     anErr = SGGetChannelDeviceList(m_vc, sgDeviceListIncludeInputs, &devices);
    if(anErr!=noErr){
        error("could not get SG channnel Device List");
    }else{
        deviceCount = (*devices)->count;
        deviceIndex = (*devices)->selectedIndex;
        logpost(NULL, 3, "SG channnel Device List count %d index %d",deviceCount,deviceIndex);
        int i;
        for (i = 0; i < deviceCount; i++){
	  logpost(NULL, 3, "SG channnel Device List  %.*s",
	       (*devices)->entry[i].name[0],
	       (*devices)->entry[i].name+1);
            }
        SGGetChannelDeviceAndInputNames(m_vc, NULL, NULL, &inputIndex);

        showInputsAsDevices = ((*devices)->entry[deviceIndex].flags) & sgDeviceNameFlagShowInputsAsDevices;

        theSGInputList = ((SGDeviceName *)(&((*devices)->entry[deviceIndex])))->inputs; //fugly

        //we should have device names in big ass undocumented structs

        //walk through the list
        //for (i = 0; i < deviceCount; i++){
        for (i = 0; i < inputIndex; i++){
            logpost(NULL, 3, "SG channnel Input Device List %d %.*s",
		 i,
		 (*theSGInputList)->entry[i].name[0],
		 (*theSGInputList)->entry[i].name+1);
        }


    }

    //this call sets the input device
    if (m_inputDevice > 0 && m_inputDevice < deviceCount) //check that the device is not out of bounds
        //anErr = SGSetChannelDeviceInput(m_vc,m_inputDevice);
        logpost(NULL, 3, "SGSetChannelDevice trying %s", 
	     (*devices)->entry[m_inputDevice].name[0],
	     (*devices)->entry[m_inputDevice].name+1);
        anErr = SGSetChannelDevice(m_vc, (*devices)->entry[m_inputDevice].name);

        if(anErr!=noErr) error("SGSetChannelDevice returned error %d",anErr);

        anErr = SGSetChannelDeviceInput(m_vc,m_inputDeviceChannel);

        if(anErr!=noErr) error("SGSetChannelDeviceInput returned error %d",anErr);

    /*  //attempt to save SG settings to disk
    NewUserData(uD);
    SGGetSettings(m_sg,uD,0);
    short uDCount;
    uDCount = CountUserDataType(*uD,sgClipType);
    post("UserDataType count %d",uDCount);

    Handle myHandle;

    PutUserDataIntoHandle(*uD,myHandle);

    int myFile;
    myFile = open("/Users/lincoln/Documents/temp",O_CREAT | O_RDWR, 0600);
    write(myFile,myHandle,4096);
    close(myFile);
    */

    //grab the VDIG info from the SGChannel
    m_vdig = SGGetVideoDigitizerComponent(m_vc);
    vdigErr = VDGetDigitizerInfo(m_vdig,&m_vdigInfo); //not sure if this is useful

    Str255    vdigName;
    memset(vdigName,0,255);
    vdigErr = VDGetInputName(m_vdig,m_inputDevice,vdigName);
    logpost(NULL, 3, "vdigName is %s",vdigName); // pascal string?

    Rect vdRect;
    vdigErr = VDGetDigitizerRect(m_vdig,&vdRect);
    logpost(NULL, 3, "digitizer rect is top %d bottom %d left %d right %d",vdRect.top,vdRect.bottom,vdRect.left,vdRect.right);

    vdigErr = VDGetActiveSrcRect(m_vdig,0,&vdRect);
    logpost(NULL, 3, "active src rect is top %d bottom %d left %d right %d",vdRect.top,vdRect.bottom,vdRect.left,vdRect.right);

    anErr = SGSetChannelBounds(m_vc, &m_srcRect);
    if(anErr!=noErr){
        error("could not set SG ChannelBounds ");
    }

    anErr = SGSetVideoRect(m_vc, &m_srcRect);
    if(anErr!=noErr){
        error("could not set SG Rect ");
    }

    anErr = SGSetChannelUsage(m_vc, seqGrabPreview);
    if(anErr!=noErr){
        error("could not set SG ChannelUsage ");
    }

    switch (m_quality){
    case 0:
        anErr = SGSetChannelPlayFlags(m_vc, channelPlayNormal);
        post("set SG NormalQuality");
        break;
    case 1:
        anErr = SGSetChannelPlayFlags(m_vc, channelPlayHighQuality);
        post("set SG HighQuality");
        break;
    case 2:
        anErr = SGSetChannelPlayFlags(m_vc, channelPlayFast);
        post("set SG FastQuality");
        break;
    case 3:
        anErr = SGSetChannelPlayFlags(m_vc, channelPlayAllData);
        post("set SG PlayAlldata");
        break;
    }
    if (m_colorspace==GL_BGRA_EXT){
      m_pixBlock.image.xsize = m_vidXSize;
      m_pixBlock.image.ysize = m_vidYSize;
      m_pixBlock.image.setCsizeByFormat(GL_RGBA_GEM);
      m_pixBlock.image.reallocate();
      m_rowBytes = m_vidXSize*4;
      anErr = QTNewGWorldFromPtr (&m_srcGWorld,
                                  k32ARGBPixelFormat,
                                  &m_srcRect,
                                  NULL,
                                  NULL,
                                  0,
                                  m_pixBlock.image.data,
                                  m_rowBytes);

      post ("using RGB");
    }else{
      m_pixBlock.image.xsize = m_vidXSize;
      m_pixBlock.image.ysize = m_vidYSize;
      m_pixBlock.image.csize = 2;
      m_pixBlock.image.format = GL_YCBCR_422_APPLE;
#ifdef __VEC__
      m_pixBlock.image.type = GL_UNSIGNED_SHORT_8_8_REV_APPLE;
#else
      m_pixBlock.image.type = GL_UNSIGNED_SHORT_8_8_APPLE;
#endif

      m_pixBlock.image.reallocate();

      m_rowBytes = m_vidXSize*2;
      anErr = QTNewGWorldFromPtr (&m_srcGWorld,
                                  //  k422YpCbCr8CodecType,

                                  k422YpCbCr8PixelFormat,
                                  // '2vuy',
                                  // kComponentVideoUnsigned,
                                  &m_srcRect,
                                  NULL,
                                  NULL,
                                  0,
                                  m_pixBlock.image.data,
                                  m_rowBytes);
      post ("using YUV");
    }

    if (anErr!= noErr)
      {
      error("%d error at QTNewGWorldFromPtr", anErr);
      return;
    }
    if (NULL == m_srcGWorld)
      {
        error("could not allocate off screen");
        return;
      }
    SGSetGWorld(m_sg,(CGrafPtr)m_srcGWorld, NULL);
    SGStartPreview(m_sg); //moved to starttransfer?
    m_haveVideo = 1;
}

void pix_videoDarwin :: setupCapture()
{
    stopTransfer();
    SGSetChannelUsage(m_vc, 0);

    if (m_record) {
        SGSetDataOutput(m_sg,&theFSSpec,seqGrabToDisk);
        switch(m_record){

        case 1:
            SGSetChannelPlayFlags(m_vc, channelPlayAllData);
            SGSetChannelUsage(m_vc, seqGrabRecord | seqGrabPlayDuringRecord);
            post("record full preview");
            break;
        case 2:
            SGSetChannelUsage(m_vc, seqGrabRecord | seqGrabPlayDuringRecord);
            post("record some preview");
            break;
        case 3:
            SGSetChannelUsage(m_vc, seqGrabRecord);
            post("record no preview");
            break;
        default:
            SGSetChannelUsage(m_vc, seqGrabRecord);
        }
    }
    else {
        SGSetChannelUsage(m_vc, seqGrabPreview);
    }
    SGUpdate(m_sg,0);

    startTransfer();
}

void pix_videoDarwin :: destroySeqGrabber()
{
    if (m_vc) {
        if (::SGDisposeChannel(m_sg, m_vc)) {
            error ("Unable to dispose a video channel");
        }
        m_vc = NULL;
    }
    if (m_sg) {
        if (::CloseComponent(m_sg)) {
            error("Unable to dispose a sequence grabber component");
        }
        m_sg = NULL;
        if (m_srcGWorld) {
        ::DisposeGWorld(m_srcGWorld);
        m_pixMap = NULL;
        m_srcGWorld = NULL;
        m_baseAddr = NULL;
    }
    }
}

void pix_videoDarwin :: resetSeqGrabber()
{
    OSErr anErr;
    post ("starting reset");

    destroySeqGrabber();
    InitSeqGrabber();
/*
    switch (m_quality){
    case 0:
        anErr = SGSetChannelPlayFlags(m_vc, channelPlayNormal);
        post("set SG NormalQuality");
        break;
    case 1:
        anErr = SGSetChannelPlayFlags(m_vc, channelPlayHighQuality);
        post("set SG HighQuality");
        break;
    case 2:
        anErr = SGSetChannelPlayFlags(m_vc, channelPlayFast);
        post("set SG FastQuality");
        break;
    case 3:
        anErr = SGSetChannelPlayFlags(m_vc, channelPlayAllData);
        post("set SG PlayAlldata");
        break;

    }
  */
}

void pix_videoDarwin :: qualityMess(int X)
{
    OSErr anErr;

    if (X < 0) X = 0;
    if (X > 3) X = 3;
    m_quality = X;

    switch (m_quality){
    case 0:
        anErr = SGSetChannelPlayFlags(m_vc, channelPlayNormal);
        post("set SG NormalQuality");
        break;
    case 1:
        anErr = SGSetChannelPlayFlags(m_vc, channelPlayHighQuality);
        post("set SG HighQuality");
        break;
    case 2:
        anErr = SGSetChannelPlayFlags(m_vc, channelPlayFast);
        post("set SG FastQuality");
        break;
    case 3:
        anErr = SGSetChannelPlayFlags(m_vc, channelPlayAllData);
        post("set SG PlayAlldata");
        break;

    }

}

void pix_videoDarwin :: derSwizzler(imageStruct &image)
{
#ifdef __VEC__

    vector unsigned char *data = (vector unsigned char*) image.data;

    vector unsigned char permuteBuffer;

    union{
        unsigned char c[16];
        vector unsigned char v;
    }cBuf;

    cBuf.c[0] = 1;
    cBuf.c[1] = 0;
    cBuf.c[2] = 3;
    cBuf.c[3] = 2;

    cBuf.c[4] = 5;
    cBuf.c[5] = 4;
    cBuf.c[6] = 7;
    cBuf.c[7] = 6;

    cBuf.c[8] = 9;
    cBuf.c[9] = 8;
    cBuf.c[10] = 11;
    cBuf.c[11] = 10;

    cBuf.c[12] = 13;
    cBuf.c[13] = 12;
    cBuf.c[14] = 15;
    cBuf.c[15] = 14;

    permuteBuffer = cBuf.v;

    int height, width, i;

    width = image.xsize / 8;
    height = image.ysize;

    i = height * width;

    while(i--){

        //vec_perm()
        data[0] = vec_perm(data[0],data[0],permuteBuffer);
        data++;

    }


#endif //__VEC__
}

/////////////////////////////////////////////////////////
// dimenMess
//
/////////////////////////////////////////////////////////
void pix_videoDarwin :: dimenMess(int x, int y, int leftmargin, int rightmargin,
    int topmargin, int bottommargin)
{
    if (x > 0 ){
        m_vidXSize = (int)x;
    }else{
        m_vidXSize = 320;
    }
    if (y > 0){
        m_vidYSize = (int)y;
    }else{
        m_vidYSize = 240;
    }
    stopTransfer();
    resetSeqGrabber();
    startTransfer();
    post("height %d width %d",m_vidYSize,m_vidXSize);
//  m_pixBlock.image.xsize = m_vidXSize;
//  m_pixBlock.image.ysize = m_vidYSize;

}

/////////////////////////////////////////////////////////
// colorspaceMess
//
/////////////////////////////////////////////////////////
void pix_videoDarwin :: csMess(int format)
{
    m_colorspace = format;
    if (format == GL_RGBA) {
     post("colorspace is GL_RGBA %d",m_colorspace);
    } else if (format == GL_BGRA_EXT) {
      post("colorspace is GL_RGBA %d",m_colorspace);
    } else if (format == GL_YCBCR_422_GEM) {
      post("colorspace is YUV %d",m_colorspace);
    } else if (format == GL_LUMINANCE) {
        post("'Gray' not yet supported...using YUV");
        format=GL_YCBCR_422_GEM;
    } else {
      error("colorspace is unknown %d", m_colorspace);
      return;
    }

    stopTransfer();
    resetSeqGrabber();
    startTransfer();
}
void pix_videoDarwin :: csMess(t_symbol*s)
{
  int format=0;
  char c =*s->s_name;
  switch (c) {
  case 'g': case 'G': format=GL_LUMINANCE; break;
  case 'y': case 'Y': format=GL_YCBCR_422_GEM; break;
  case 'r': case 'R': format=GL_RGBA; break;
  default:
    post("colorspace must be 'RGBA', 'YUV' or 'Gray'");
    return;
  }

  csMess(format);
}

void pix_videoDarwin :: fileMess(int argc, t_atom *argv)
{
    OSErr        err = noErr;
    FSRef        ref;

//if recording is going do not accept a new file name
//on OSX changing the name while recording won't have any effect
//but it will give the wrong message at the end if recording
//if (m_recordStart) return;

//  char *extension = ".mov";
  if (argc) {
    if (argv->a_type == A_SYMBOL) {
      atom_string(argv++, m_filename, 80);
      argc--;
     // sprintf(m_filename, "%s", m_pathname);
    }
   // if (argc>0)
  //    m_filetype = atom_getint(argv);
  }

 // m_autocount = 0;
    setModified();

    post("filename %s",m_filename);

    if (!m_filename[0]) {
        error("no filename passed");
        return;
        } else {
            err = ::FSPathMakeRef((UInt8*)m_filename, &ref, NULL);
            if (err == fnfErr) {
                // if the file does not yet exist, then let's create the file
                int fd;
                fd = open(m_filename, O_CREAT | O_RDWR, 0600);
                if (fd < 0){
                    error("problem with fd");
                    return ;
                    }
                        write(fd, " ", 1);
                        close(fd);
                        err = FSPathMakeRef((UInt8*)m_filename, &ref, NULL);
                        //post("made new file %s",m_filename);
            }


            if (err) {
                error("unable to make file ref from filename %s", m_filename);
                return;
            }

            //err = ::FSsetCatalogInfo(&ref, kFSCatInfoSettableInfo, NULL);
            err = FSGetCatalogInfo(&ref, kFSCatInfoNodeFlags, NULL, NULL, &theFSSpec, NULL);

            if (err != noErr){
                    error("error %d in FSGetCatalogInfo()", err);
                    return;
                }


        //    err = FSMakeFSSpec(theFSSpec.vRefNum, theFSSpec.parID, (UInt8*)m_filename, &theFSSpec);

            if (err != noErr && err != -37){
                    error("error %d in FSMakeFSSpec()", err);
                    return;
                }

        }

    CreateMovieFile(    &theFSSpec,
                            FOUR_CHAR_CODE('TVOD'),
                            smSystemScript,
                            createMovieFileDontOpenFile |
                            createMovieFileDontCreateMovie |
                            createMovieFileDontCreateResFile,
                            NULL,
                            NULL);

}


void pix_videoDarwin :: brightnessMess(float X)
{

    QTAtomContainer         atomContainer;
    QTAtom                  featureAtom;
    VDIIDCFeatureSettings   settings;
    ComponentDescription    desc;
    ComponentResult         result = paramErr;

    //check if device is IIDC
    GetComponentInfo((Component)m_vdig, &desc, NULL, NULL, NULL);
    if (vdSubtypeIIDC != desc.componentSubType){

        m_brightness = (unsigned short)(65536. * X);
        VDSetBrightness(m_vdig,&m_brightness);

        VDGetBrightness(m_vdig,&m_brightness);
        post("brightness is %d",m_brightness);
    }
    else
    {
    //IIDC stuff
    //these things are as stubborn as they are stupid - find one that conforms to spec!

    //vdIIDCFeatureBrightness
    result = VDIIDCGetFeaturesForSpecifier(m_vdig, vdIIDCFeatureBrightness, &atomContainer);
    if (noErr != result) {
        error("VDIIDCGetFeaturesForSpecifier returned %d",result);
    }

    featureAtom = QTFindChildByIndex(atomContainer, kParentAtomIsContainer,
                                         vdIIDCAtomTypeFeature, 1, NULL);
    if (0 == featureAtom) error("featureAtom not found");

    result = QTCopyAtomDataToPtr(atomContainer,
                                     QTFindChildByID(atomContainer, featureAtom,
                                     vdIIDCAtomTypeFeatureSettings,
                                     vdIIDCAtomIDFeatureSettings, NULL),
                                     true, sizeof(settings), &settings, NULL);

    settings.state.flags = (vdIIDCFeatureFlagOn |
                                        vdIIDCFeatureFlagManual |
                                        vdIIDCFeatureFlagRawControl);

    settings.state.value = X;

    result = QTSetAtomData(atomContainer,
                                       QTFindChildByID(atomContainer, featureAtom,
                                       vdIIDCAtomTypeFeatureSettings,
                                       vdIIDCAtomIDFeatureSettings, NULL),
                                       sizeof(settings), &settings);

    result = VDIIDCSetFeatures(m_vdig, atomContainer);

    }
}

void pix_videoDarwin :: saturationMess(float X)
{

    QTAtomContainer         atomContainer;
    QTAtom                  featureAtom;
    VDIIDCFeatureSettings   settings;
    ComponentDescription    desc;
    ComponentResult         result = paramErr;

    //check if device is IIDC
    GetComponentInfo((Component)m_vdig, &desc, NULL, NULL, NULL);
    if (vdSubtypeIIDC != desc.componentSubType){

        m_saturation = (unsigned short)(65536. * X);
        VDSetSaturation(m_vdig,&m_saturation);

        VDGetSaturation(m_vdig,&m_saturation);
        post("saturation is %d",m_saturation);
    }
    else
    {
    //IIDC stuff
    //vdIIDCFeatureSaturation
    result = VDIIDCGetFeaturesForSpecifier(m_vdig, vdIIDCFeatureSaturation, &atomContainer);
    if (noErr != result) {
        error("VDIIDCGetFeaturesForSpecifier vdIIDCFeatureSaturation returned %d",result);
    }

    featureAtom = QTFindChildByIndex(atomContainer, kParentAtomIsContainer,
                                         vdIIDCAtomTypeFeature, 1, NULL);
    if (0 == featureAtom) error("featureAtom vdIIDCFeatureSaturation not found");

    result = QTCopyAtomDataToPtr(atomContainer,
                                     QTFindChildByID(atomContainer, featureAtom,
                                     vdIIDCAtomTypeFeatureSettings,
                                     vdIIDCAtomIDFeatureSettings, NULL),
                                     true, sizeof(settings), &settings, NULL);

    settings.state.flags = (vdIIDCFeatureFlagOn |
                                        vdIIDCFeatureFlagManual |
                                        vdIIDCFeatureFlagRawControl);

    settings.state.value = X;

    result = QTSetAtomData(atomContainer,
                                       QTFindChildByID(atomContainer, featureAtom,
                                       vdIIDCAtomTypeFeatureSettings,
                                       vdIIDCAtomIDFeatureSettings, NULL),
                                       sizeof(settings), &settings);

    result = VDIIDCSetFeatures(m_vdig, atomContainer);

    }
}

void pix_videoDarwin :: contrastMess(float X)
{

        m_contrast = (unsigned short)(65536. * X);
        VDSetContrast(m_vdig,&m_contrast);

        VDGetContrast(m_vdig,&m_contrast);
        post("contrast is %d",m_contrast);
}

void pix_videoDarwin :: exposureMess(float X)
{

    QTAtomContainer         atomContainer;
    QTAtom                  featureAtom;
    VDIIDCFeatureSettings   settings;
    ComponentDescription    desc;
    ComponentResult         result = paramErr;

    //check if device is IIDC
    GetComponentInfo((Component)m_vdig, &desc, NULL, NULL, NULL);
    if (vdSubtypeIIDC == desc.componentSubType){

    //IIDC stuff
    //vdIIDCFeatureExposure
    result = VDIIDCGetFeaturesForSpecifier(m_vdig, vdIIDCFeatureExposure, &atomContainer);
    if (noErr != result) {
        error("VDIIDCGetFeaturesForSpecifier vdIIDCFeatureExposure returned %d",result);
    }

    featureAtom = QTFindChildByIndex(atomContainer, kParentAtomIsContainer,
                                         vdIIDCAtomTypeFeature, 1, NULL);
    if (0 == featureAtom) error("featureAtom vdIIDCFeatureExposure not found");

    result = QTCopyAtomDataToPtr(atomContainer,
                                     QTFindChildByID(atomContainer, featureAtom,
                                     vdIIDCAtomTypeFeatureSettings,
                                     vdIIDCAtomIDFeatureSettings, NULL),
                                     true, sizeof(settings), &settings, NULL);

    settings.state.flags = (vdIIDCFeatureFlagOn |
                                        vdIIDCFeatureFlagManual |
                                        vdIIDCFeatureFlagRawControl);

    settings.state.value = X;

    result = QTSetAtomData(atomContainer,
                                       QTFindChildByID(atomContainer, featureAtom,
                                       vdIIDCAtomTypeFeatureSettings,
                                       vdIIDCAtomIDFeatureSettings, NULL),
                                       sizeof(settings), &settings);

    result = VDIIDCSetFeatures(m_vdig, atomContainer);

    }
}

void pix_videoDarwin :: gainMess(float X)
{

    QTAtomContainer         atomContainer;
    QTAtom                  featureAtom;
    VDIIDCFeatureSettings   settings;
    ComponentDescription    desc;
    ComponentResult         result = paramErr;

    //check if device is IIDC
    GetComponentInfo((Component)m_vdig, &desc, NULL, NULL, NULL);
    if (vdSubtypeIIDC == desc.componentSubType){

    //IIDC stuff
    //vdIIDCFeatureGain
    result = VDIIDCGetFeaturesForSpecifier(m_vdig, vdIIDCFeatureWhiteBalanceU, &atomContainer);
    if (noErr != result) {
        error("VDIIDCGetFeaturesForSpecifier vdIIDCFeatureExposure returned %d",result);
    }

    featureAtom = QTFindChildByIndex(atomContainer, kParentAtomIsContainer,
                                         vdIIDCAtomTypeFeature, 1, NULL);
    if (0 == featureAtom) error("featureAtom vdIIDCFeatureExposure not found");

    result = QTCopyAtomDataToPtr(atomContainer,
                                     QTFindChildByID(atomContainer, featureAtom,
                                     vdIIDCAtomTypeFeatureSettings,
                                     vdIIDCAtomIDFeatureSettings, NULL),
                                     true, sizeof(settings), &settings, NULL);

    settings.state.flags = (vdIIDCFeatureFlagOn |
                                        vdIIDCFeatureFlagManual |
                                        vdIIDCFeatureFlagRawControl);

    settings.state.value = X;

    result = QTSetAtomData(atomContainer,
                                       QTFindChildByID(atomContainer, featureAtom,
                                       vdIIDCAtomTypeFeatureSettings,
                                       vdIIDCAtomIDFeatureSettings, NULL),
                                       sizeof(settings), &settings);

    result = VDIIDCSetFeatures(m_vdig, atomContainer);

    }
}

void pix_videoDarwin :: whiteBalanceMess(float U, float V)
{

    QTAtomContainer         atomContainer;
    QTAtom                  featureAtom;
    VDIIDCFeatureSettings   settings;
    ComponentDescription    desc;
    ComponentResult         result = paramErr;

    //check if device is IIDC
    GetComponentInfo((Component)m_vdig, &desc, NULL, NULL, NULL);
    if (vdSubtypeIIDC == desc.componentSubType){

    //IIDC stuff
    //vdIIDCFeatureWhiteBalanceU
    result = VDIIDCGetFeaturesForSpecifier(m_vdig, vdIIDCFeatureWhiteBalanceU, &atomContainer);
    if (noErr != result) {
        error("VDIIDCGetFeaturesForSpecifier vdIIDCFeatureExposure returned %d",result);
    }

    featureAtom = QTFindChildByIndex(atomContainer, kParentAtomIsContainer,
                                         vdIIDCAtomTypeFeature, 1, NULL);
    if (0 == featureAtom) error("featureAtom vdIIDCFeatureExposure not found");

    result = QTCopyAtomDataToPtr(atomContainer,
                                     QTFindChildByID(atomContainer, featureAtom,
                                     vdIIDCAtomTypeFeatureSettings,
                                     vdIIDCAtomIDFeatureSettings, NULL),
                                     true, sizeof(settings), &settings, NULL);

    settings.state.flags = (vdIIDCFeatureFlagOn |
                                        vdIIDCFeatureFlagManual |
                                        vdIIDCFeatureFlagRawControl);

    settings.state.value = U;

    result = QTSetAtomData(atomContainer,
                                       QTFindChildByID(atomContainer, featureAtom,
                                       vdIIDCAtomTypeFeatureSettings,
                                       vdIIDCAtomIDFeatureSettings, NULL),
                                       sizeof(settings), &settings);

    result = VDIIDCSetFeatures(m_vdig, atomContainer);

    //vdIIDCFeatureWhiteBalanceV
    result = VDIIDCGetFeaturesForSpecifier(m_vdig, vdIIDCFeatureWhiteBalanceV, &atomContainer);
    if (noErr != result) {
        error("VDIIDCGetFeaturesForSpecifier vdIIDCFeatureExposure returned %d",result);
    }

    featureAtom = QTFindChildByIndex(atomContainer, kParentAtomIsContainer,
                                         vdIIDCAtomTypeFeature, 1, NULL);
    if (0 == featureAtom) error("featureAtom vdIIDCFeatureExposure not found");

    result = QTCopyAtomDataToPtr(atomContainer,
                                     QTFindChildByID(atomContainer, featureAtom,
                                     vdIIDCAtomTypeFeatureSettings,
                                     vdIIDCAtomIDFeatureSettings, NULL),
                                     true, sizeof(settings), &settings, NULL);

    settings.state.flags = (vdIIDCFeatureFlagOn |
                                        vdIIDCFeatureFlagManual |
                                        vdIIDCFeatureFlagRawControl);

    settings.state.value = V;

    result = QTSetAtomData(atomContainer,
                                       QTFindChildByID(atomContainer, featureAtom,
                                       vdIIDCAtomTypeFeatureSettings,
                                       vdIIDCAtomIDFeatureSettings, NULL),
                                       sizeof(settings), &settings);

    result = VDIIDCSetFeatures(m_vdig, atomContainer);


    }
}

/////////////////////////////////////////////////////////
// dialog
//
/////////////////////////////////////////////////////////
void pix_videoDarwin :: dialogMess(int argc, t_atom*argv)
{
    DoVideoSettings();
}

void pix_videoDarwin :: obj_setupCallback(t_class *classPtr)
{
    class_addcreator(reinterpret_cast<t_newmethod>(create_pix_videoDarwin),
                      gensym("pix_video"),
                      A_DEFFLOAT,A_DEFFLOAT,A_NULL);

    pix_videoOS::real_obj_setupCallback(classPtr);

    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_videoDarwin::qualityCallback),
          gensym("quality"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_videoDarwin::resetCallback),
          gensym("reset"), A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_videoDarwin::autoCallback),
          gensym("auto"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_videoDarwin::deviceCallback),
                              gensym("device"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_videoDarwin::brightnessCallback),
                              gensym("brightness"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_videoDarwin::saturationCallback),
                              gensym("saturation"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_videoDarwin::contrastCallback),
                              gensym("contrast"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_videoDarwin::exposureCallback),
                              gensym("exposure"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_videoDarwin::gainCallback),
                              gensym("gain"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_videoDarwin::whiteBalanceCallback),
                              gensym("whitebalance"), A_DEFFLOAT, A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_videoDarwin::fileMessCallback),
          gensym("file"), A_GIMME, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_videoDarwin::recordCallback),
          gensym("record"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_videoDarwin::inputCallback),
          gensym("input"), A_DEFFLOAT, A_NULL);

    class_addbang(classPtr,reinterpret_cast<t_method>(&pix_videoDarwin::bangMessCallback));
}

void pix_videoDarwin :: qualityCallback(void *data, t_floatarg X)
{
    GetMyClass(data)->qualityMess((int)X);
}

void pix_videoDarwin :: autoCallback(void *data, t_floatarg X)
{
    GetMyClass(data)->m_auto=((int)X);
}

void pix_videoDarwin :: recordCallback(void *data, t_floatarg X)
{
    GetMyClass(data)->m_record=((int)X);
    GetMyClass(data)->setupCapture();
}


void pix_videoDarwin :: bangMessCallback(void *data)
{
    GetMyClass(data)->m_banged=true;
}

void pix_videoDarwin :: resetCallback(void *data)
{
    GetMyClass(data)->resetSeqGrabber();
}

void pix_videoDarwin :: deviceCallback(void *data, t_floatarg X)
{
  GetMyClass(data)->m_inputDevice=((int)X);
}

void pix_videoDarwin :: inputCallback(void *data, t_floatarg X)
{
  GetMyClass(data)->m_inputDeviceChannel=((int)X);
}

void pix_videoDarwin :: brightnessCallback(void *data, t_floatarg X)
{
  GetMyClass(data)->brightnessMess(X);
}

void pix_videoDarwin :: saturationCallback(void *data, t_floatarg X)
{
  GetMyClass(data)->saturationMess(X);
}

void pix_videoDarwin :: contrastCallback(void *data, t_floatarg X)
{
  GetMyClass(data)->contrastMess(X);
}

void pix_videoDarwin :: exposureCallback(void *data, t_floatarg X)
{
  GetMyClass(data)->exposureMess(X);
}

void pix_videoDarwin :: gainCallback(void *data, t_floatarg X)
{
  GetMyClass(data)->gainMess(X);
}

void pix_videoDarwin :: whiteBalanceCallback(void *data, t_floatarg U, t_floatarg V)
{
  GetMyClass(data)->whiteBalanceMess(U,V);
}

void pix_videoDarwin :: fileMessCallback(void *data, t_symbol *s, int argc, t_atom *argv)
{
  GetMyClass(data)->fileMess(argc, argv);
}

#endif // __APPLE__
