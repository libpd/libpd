//------------------------------------------------------------------------------
// File: Grabber.cpp
//
// Desc: DirectShow sample code - Implementation file for the SampleGrabber
//       example filter
//
// Copyright (c) 1997-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_DIRECTSHOW

#include <streams.h>     // Active Movie (includes windows.h)
#include <initguid.h>    // declares DEFINE_GUID to declare an EXTERN_C const.

#include "qedit.h"
#include "DSgrabber.h"

#pragma warning(disable: 4800)

// Local helper functions
IPin * GetInPin( IBaseFilter * pFilter, int PinNum );
IPin * GetOutPin( IBaseFilter * pFilter, int PinNum );


// Setup data - allows the self-registration to work.
const AMOVIESETUP_MEDIATYPE sudPinTypes =
{ &MEDIATYPE_NULL        // clsMajorType
, &MEDIASUBTYPE_NULL };  // clsMinorType


const AMOVIESETUP_PIN psudSampleGrabberPins[] =
{ { L"Input"            // strName
  , FALSE               // bRendered
  , FALSE               // bOutput
  , FALSE               // bZero
  , FALSE               // bMany
  , &CLSID_NULL         // clsConnectsToFilter
  , L""                 // strConnectsToPin
  , 1                   // nTypes
  , &sudPinTypes        // lpTypes
  }
, { L"Output"           // strName
  , FALSE               // bRendered
  , TRUE                // bOutput
  , FALSE               // bZero
  , FALSE               // bMany
  , &CLSID_NULL         // clsConnectsToFilter
  , L""                 // strConnectsToPin
  , 1                   // nTypes
  , &sudPinTypes        // lpTypes
  }
};

const AMOVIESETUP_FILTER sudSampleGrabber =
{ &CLSID_GrabberSample            // clsID
, L"SampleGrabber Example"        // strName
, MERIT_DO_NOT_USE                // dwMerit
, 2                               // nPins
, psudSampleGrabberPins };        // lpPin


// Needed for the CreateInstance mechanism
CFactoryTemplate g_Templates[]=
{
    { L"Sample Grabber Example"
        , &CLSID_GrabberSample
        , CSampleGrabber::CreateInstance
        , NULL
        , &sudSampleGrabber }

};

int g_cTemplates = sizeof(g_Templates)/sizeof(g_Templates[0]);


/******************************Public*Routine******************************\
*
* Exported entry points for registration and unregistration (in this case 
* they only call through to default implmentations).
*
*\**************************************************************************/
STDAPI
DllRegisterServer() {
    return AMovieDllRegisterServer2(TRUE);
}

STDAPI
DllUnregisterServer() {
    return AMovieDllRegisterServer2(FALSE);
}

//
// CreateInstance
//
// Provide the way for COM to create a CSampleGrabber object
//
CUnknown * WINAPI CSampleGrabber::CreateInstance(LPUNKNOWN punk, HRESULT *phr) 
{
    // assuming we don't want to modify the data
    CSampleGrabber *pNewObject = new CSampleGrabber(punk, phr, FALSE);

    if(pNewObject == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return pNewObject;   
} // CreateInstance


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

CSampleGrabber::CSampleGrabber( IUnknown * pOuter, HRESULT * phr, BOOL ModifiesData )
: CTransInPlaceFilter( TEXT("SampleGrabber"), (IUnknown*) pOuter, 
                       CLSID_GrabberSample, phr, (BOOL)ModifiesData )
, m_callback( NULL )
{
    // this is used to override the input pin with our own
    
    m_pInput = (CTransInPlaceInputPin*) new CSampleGrabberInPin( this, phr );
    if( !m_pInput )
    {
        *phr = E_OUTOFMEMORY;
    }
}

STDMETHODIMP CSampleGrabber::NonDelegatingQueryInterface( REFIID riid, void ** ppv) 
{
    if(riid == IID_IGrabberSample) {                
        return GetInterface((IGrabberSample *) this, ppv);
    }
    else {
        return CTransInPlaceFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}


//----------------------------------------------------------------------------
// this is where you force the sample grabber to connect with one type
// or the other. What you do here is crucial to what type of data your
// app will be dealing with in the sample grabber's callback. For instance,
// if you don't enforce right-side-up video in this call, you may not get
// right-side-up video in your callback. It all depends on what you do here.
//----------------------------------------------------------------------------

HRESULT CSampleGrabber::CheckInputType( const CMediaType * pmt )
{
    CAutoLock lock( &m_Lock );

    // if the major type's not set, then accept any old thing

    GUID g;
    g = *m_mtAccept.Type( );
    if( g == GUID_NULL )
    {
        return NOERROR;
    }

    // if the major type is set, don't accept anything else

    if( g != *pmt->Type( ) )
    {
        return VFW_E_INVALID_MEDIA_TYPE;
    }

    // subtypes must match, if set. if not set, accept anything

    g = *m_mtAccept.Subtype( );
    if( g == GUID_NULL )
    {
        return NOERROR;
    }
    if( g != *pmt->Subtype( ) )
    {
        return VFW_E_INVALID_MEDIA_TYPE;
    }

    // format types must match, if one is set

    g = *m_mtAccept.FormatType( );
    if( g == GUID_NULL )
    {
        return NOERROR;
    }
    if( g != *pmt->FormatType( ) )
    {
        return VFW_E_INVALID_MEDIA_TYPE;
    }

    // at this point, for this sample code, this is good enough,
    // but you may want to make it more strict

    return NOERROR;
}

//----------------------------------------------------------------------------
// this bit is almost straight out of the base classes.
// We override this so we can handle Transform( )'s error
// result differently.
//----------------------------------------------------------------------------

HRESULT CSampleGrabber::Receive( IMediaSample * pms )
{
    AM_SAMPLE2_PROPERTIES * const pProps = m_pInput->SampleProps();
    if (pProps->dwStreamId != AM_STREAM_MEDIA) 
    {
        if( m_pOutput->IsConnected() )
            return m_pOutput->Deliver(pms);
        else
            return NOERROR;
    }

    HRESULT hr;

    if (UsingDifferentAllocators()) 
    {
        // We have to copy the data.

        pms = Copy(pms);

        if (pms==NULL) 
        {
            return E_UNEXPECTED;
        }
    }

    // have the derived class transform the data
    hr = Transform(pms);

    if (FAILED(hr)) 
    {
        DbgLog((LOG_TRACE, 1, TEXT("Error from TransInPlace")));
        if (UsingDifferentAllocators()) 
        {
            pms->Release();
        }
        return hr;
    }

    if (hr == NOERROR) 
    {
        hr = m_pOutput->Deliver(pms);
    }
    
    // release the output buffer. If the connected pin still needs it,
    // it will have addrefed it itself.
    if (UsingDifferentAllocators()) 
    {
        pms->Release();
    }

    return hr;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HRESULT CSampleGrabber::Transform( IMediaSample * pms )
{
    CAutoLock lock( &m_Lock );

    if( m_callback )
    {
        REFERENCE_TIME StartTime, StopTime;
        pms->GetTime( &StartTime, &StopTime);
        StartTime += m_pInput->CurrentStartTime( );
        StopTime += m_pInput->CurrentStartTime( );

        BOOL * pTypeChanged = &((CSampleGrabberInPin*)m_pInput)->m_bMediaTypeChanged;

        HRESULT hr = m_callback( m_pUser, pms, &StartTime, &StopTime, *pTypeChanged );

        *pTypeChanged = FALSE; // now that we notified user, we can clear it

        return hr;
    }

    return NOERROR;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HRESULT CSampleGrabber::SetAcceptedMediaType( const CMediaType * pmt )
{
    CAutoLock lock( &m_Lock );

    if( !pmt )
    {
        m_mtAccept = CMediaType( );
        return NOERROR;        
    }

    HRESULT hr;
    hr = CopyMediaType( &m_mtAccept, pmt );

    return hr;
}


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HRESULT CSampleGrabber::GetConnectedMediaType( CMediaType * pmt )
{
    CAutoLock lock( &m_Lock );

    HRESULT hr;
    hr = CopyMediaType( pmt, &InputPin( )->CurrentMediaType() );

    return hr;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HRESULT CSampleGrabber::SetCallback( SAMPLECALLBACK Callback, void* pUser )
{
    CAutoLock lock( &m_Lock );

    m_callback = Callback;
	m_pUser = pUser;

    return NOERROR;
}

//----------------------------------------------------------------------------
// inform the input pin of the allocator buffer we wish to use. See the
// input pin's SetDeliverBuffer method for comments. 
//----------------------------------------------------------------------------

HRESULT CSampleGrabber::SetDeliveryBuffer( ALLOCATOR_PROPERTIES props, BYTE * m_pBuffer )
{
    // they can't be connected if we're going to be changing delivery buffers
    //
    if( InputPin( )->IsConnected( ) || OutputPin( )->IsConnected( ) )
    {
        return E_INVALIDARG;
    }
    return ((CSampleGrabberInPin*)m_pInput)->SetDeliveryBuffer( props, m_pBuffer );
}

//----------------------------------------------------------------------------
// used to help speed input pin connection times. We return a partially
// specified media type - only the main type is specified. If we return
// anything BUT a major type, some codecs written improperly will crash
//----------------------------------------------------------------------------

HRESULT CSampleGrabberInPin::GetMediaType( int iPosition, CMediaType * pMediaType )
{
    if (iPosition < 0) {
        return E_INVALIDARG;
    }
    if (iPosition > 0) {
        return VFW_S_NO_MORE_ITEMS;
    }

    *pMediaType = CMediaType( );
    pMediaType->SetType( ((CSampleGrabber*)m_pFilter)->m_mtAccept.Type( ) );
    return S_OK;
}

//----------------------------------------------------------------------------
// override the CTransInPlaceInputPin's method, and return a new enumerator
// if the input pin is disconnected. This will allow GetMediaType to be
// called. If we didn't do this, EnumMediaTypes returns a failure code
// and GetMediaType is never called. 
//----------------------------------------------------------------------------

STDMETHODIMP CSampleGrabberInPin::EnumMediaTypes( IEnumMediaTypes **ppEnum )
{
    CheckPointer(ppEnum,E_POINTER);
    ValidateReadWritePtr(ppEnum,sizeof(IEnumMediaTypes *));

    // if the output pin isn't connected yet, offer the possibly 
    // partially specified media type that has been set by the user

    if( !((CSampleGrabber*)m_pTIPFilter)->OutputPin( )->IsConnected() )
    {
        /* Create a new ref counted enumerator */

        *ppEnum = new CEnumMediaTypes( this, NULL );

        return (*ppEnum) ? NOERROR : E_OUTOFMEMORY;
    }

    // if the output pin is connected, offer it's fully qualified media type

    return ((CSampleGrabber*)m_pTIPFilter)->OutputPin( )->GetConnected()->EnumMediaTypes( ppEnum );

}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

STDMETHODIMP CSampleGrabberInPin::NotifyAllocator( IMemAllocator *pAllocator, BOOL bReadOnly )
{
    if( m_pPrivateAllocator )
    {
        if( pAllocator != m_pPrivateAllocator )
        {
            return E_FAIL;
        }
        else
        {
            // if the upstream guy wants to be read only and we don't, then that's bad
            // if the upstream guy doesn't request read only, but we do, that's okay
            if( bReadOnly && !SampleGrabber( )->IsReadOnly( ) )
            {
                return E_FAIL;
            }
        }
    }

    return CTransInPlaceInputPin::NotifyAllocator( pAllocator, bReadOnly );
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

STDMETHODIMP CSampleGrabberInPin::GetAllocator( IMemAllocator **ppAllocator )
{
    if( m_pPrivateAllocator )
    {
        *ppAllocator = m_pPrivateAllocator;
        m_pPrivateAllocator->AddRef( );
        return NOERROR;
    }
    else
    {
        return CTransInPlaceInputPin::GetAllocator( ppAllocator );
    }
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HRESULT CSampleGrabberInPin::SetDeliveryBuffer( ALLOCATOR_PROPERTIES props, BYTE * pBuffer )
{
    // don't allow more than one buffer

    if( props.cBuffers != 1 )
    {
        return E_INVALIDARG;
    }
    if( !pBuffer )
    {
        return E_POINTER;
    }

    m_allocprops = props;
    m_pBuffer = pBuffer;

    HRESULT hr = 0;
    m_pPrivateAllocator = new CSampleGrabberAllocator( this, &hr );
    if( !m_pPrivateAllocator )
    {
        return E_OUTOFMEMORY;
    }
    m_pPrivateAllocator->AddRef( );
    return hr;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HRESULT CSampleGrabberInPin::SetMediaType( const CMediaType *pmt )
{
    m_bMediaTypeChanged = TRUE;

    return CTransInPlaceInputPin::SetMediaType( pmt );
}

//----------------------------------------------------------------------------
// ask for the allocator props. this can hardly go wrong
//----------------------------------------------------------------------------

HRESULT CSampleGrabberAllocator::GetAllocatorRequirements( ALLOCATOR_PROPERTIES *pProps )
{
    *pProps = m_pPin->m_allocprops;
    return NOERROR;
}

//----------------------------------------------------------------------------
// don't allocate the memory, just use the buffer the app set up
//----------------------------------------------------------------------------

HRESULT CSampleGrabberAllocator::Alloc( )
{
    // look at the base class code to see where this came from!

    CAutoLock lck(this);

    /* Check he has called SetProperties */
    HRESULT hr = CBaseAllocator::Alloc();
    if (FAILED(hr)) {
        return hr;
    }

    /* If the requirements haven't changed then don't reallocate */
    if (hr == S_FALSE) {
        ASSERT(m_pBuffer);
        return NOERROR;
    }
    ASSERT(hr == S_OK); // we use this fact in the loop below

    /* Free the old resources */
    if (m_pBuffer) {
        ReallyFree();
    }

    /* Compute the aligned size */
    LONG lAlignedSize = m_lSize + m_lPrefix;
    if (m_lAlignment > 1) {
        LONG lRemainder = lAlignedSize % m_lAlignment;
        if (lRemainder != 0) {
            lAlignedSize += (m_lAlignment - lRemainder);
        }
    }

    /* Create the contiguous memory block for the samples
       making sure it's properly aligned (64K should be enough!)
    */
    ASSERT(lAlignedSize % m_lAlignment == 0);

    // don't create the buffer - use what was passed to us
    //
    m_pBuffer = m_pPin->m_pBuffer;

    if (m_pBuffer == NULL) {
        return E_OUTOFMEMORY;
    }

    LPBYTE pNext = m_pBuffer;
    CMediaSample *pSample;

    ASSERT(m_lAllocated == 0);

    // Create the new samples - we have allocated m_lSize bytes for each sample
    // plus m_lPrefix bytes per sample as a prefix. We set the pointer to
    // the memory after the prefix - so that GetPointer() will return a pointer
    // to m_lSize bytes.
    for (; m_lAllocated < m_lCount; m_lAllocated++, pNext += lAlignedSize) {

        pSample = new CMediaSample(
                        NAME("Sample Grabber memory media sample"),
                        this,
                        &hr,
                        pNext + m_lPrefix,      // GetPointer() value
                        m_lSize);               // not including prefix

        ASSERT(SUCCEEDED(hr));
        if (pSample == NULL) {
            return E_OUTOFMEMORY;
        }

        // This CANNOT fail
        m_lFree.Add(pSample);
    }

    m_bChanged = FALSE;
    return NOERROR;
}

//----------------------------------------------------------------------------
// don't really free the memory
//----------------------------------------------------------------------------

void CSampleGrabberAllocator::ReallyFree()
{
    // look at the base class code to see where this came from!

    /* Should never be deleting this unless all buffers are freed */

    ASSERT(m_lAllocated == m_lFree.GetCount());

    /* Free up all the CMediaSamples */

    CMediaSample *pSample;
    for (;;) {
        pSample = m_lFree.RemoveHead();
        if (pSample != NULL) {
            delete pSample;
        } else {
            break;
        }
    }

    m_lAllocated = 0;

    // don't free the buffer - let the app do it
}


IPin * GetInPin( IBaseFilter * pFilter, int PinNum )
{
    IEnumPins * pEnum = 0;
    HRESULT hr = pFilter->EnumPins( &pEnum );
    pEnum->Reset( );
    ULONG Fetched;
    do
    {
        Fetched = 0;
        IPin * pPin = 0;
        pEnum->Next( 1, &pPin, &Fetched );
        if( Fetched )
        {
            PIN_DIRECTION pd;
            pPin->QueryDirection( &pd);
            pPin->Release( );
            if( pd == PINDIR_INPUT )
            {
                if( PinNum == 0 )
                {
                    pEnum->Release( );
                    return pPin;
                }
                PinNum--;
            }
        }
    }
    while( Fetched );
    pEnum->Release( );
    return NULL;
}

IPin * GetOutPin( IBaseFilter * pFilter, int PinNum )
{
    IEnumPins * pEnum = 0;
    HRESULT hr = pFilter->EnumPins( &pEnum );
    pEnum->Reset( );
    ULONG Fetched;
    do
    {
        Fetched = 0;
        IPin * pPin = 0;
        pEnum->Next( 1, &pPin, &Fetched );
        if( Fetched )
        {
            PIN_DIRECTION pd;
            pPin->QueryDirection( &pd);
            pPin->Release( );
            if( pd == PINDIR_OUTPUT )
            {
                if( PinNum == 0 )
                {
                    pEnum->Release( );
                    return pPin;
                }
                PinNum--;
            }
        }
    }
    while( Fetched );
    pEnum->Release( );
    return NULL;
}

#endif //HAVE_DIRECTSHOW
