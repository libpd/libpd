//------------------------------------------------------------------------------
// File: Grabber.h
//
// Desc: DirectShow sample code - Header file for the SampleGrabber
//       example filter
//
// Copyright (c) 1997-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Define GUID for the sample grabber example so that it does NOT
// conflict with the official DirectX SampleGrabber filter
//------------------------------------------------------------------------------
// {2FA4F053-6D60-4cb0-9503-8E89234F3F73}
DEFINE_GUID(CLSID_GrabberSample, 
0x2fa4f053, 0x6d60, 0x4cb0, 0x95, 0x3, 0x8e, 0x89, 0x23, 0x4f, 0x3f, 0x73);

// we define a callback typedef for this example. 
// Normally, you would make the SampleGrabber 
// support a COM interface, and in one of it's methods
// you would pass in a pointer to a COM interface 
// for calling back. See the 
// DX8 documentation for the SampleGrabber

typedef HRESULT (*SAMPLECALLBACK) (
	void*			pUser,
    IMediaSample * pSample, 
    REFERENCE_TIME * StartTime, 
    REFERENCE_TIME * StopTime,
    BOOL TypeChanged );


DEFINE_GUID(IID_IGrabberSample, 
0x6b652fff, 0x11fe, 0x4fce, 0x92, 0xad, 0x02, 0x66, 0xb5, 0xd7, 0xc7, 0x8f);


// we define the interface the app can use to program us
MIDL_INTERFACE("6B652FFF-11FE-4fce-92AD-0266B5D7C78F")
IGrabberSample : public IUnknown
    {
    public:
        
        virtual HRESULT STDMETHODCALLTYPE SetAcceptedMediaType( 
            const CMediaType *pType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType( 
            CMediaType * pmt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCallback( 
            SAMPLECALLBACK Callback, void* pUser) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDeliveryBuffer( 
            ALLOCATOR_PROPERTIES props,
            BYTE *pBuffer) = 0;
    };
        

class CSampleGrabberInPin;
class CSampleGrabber;

//----------------------------------------------------------------------------
// this is a special allocator that KNOWS the person who is creating it
// will only create one of them. It allocates CMediaSamples that only 
// reference the buffer location that is set in the pin's renderer's
// data variable
//----------------------------------------------------------------------------

class CSampleGrabberAllocator : public CMemAllocator
{
    friend class CSampleGrabberInPin;
    friend class CSampleGrabber;

protected:

    // our pin who created us
    //
    CSampleGrabberInPin * m_pPin;

public:

    CSampleGrabberAllocator( CSampleGrabberInPin * pParent, HRESULT *phr ) 
        : CMemAllocator( TEXT("SampleGrabberAllocator"), NULL, phr ) 
        , m_pPin( pParent )
    {
    };

    ~CSampleGrabberAllocator( )
    {
        // wipe out m_pBuffer before we try to delete it. It's not an allocated
        // buffer, and the default destructor will try to free it!
        m_pBuffer = NULL;
    }

    // we override this to tell whoever's upstream of us what kind of
    // properties we're going to demand to have
    //
    HRESULT GetAllocatorRequirements( ALLOCATOR_PROPERTIES *pProps );

    HRESULT Alloc( );

    void ReallyFree();
};

//----------------------------------------------------------------------------
// we override the input pin class so we can provide a media type
// to speed up connection times. When you try to connect a filesourceasync
// to a transform filter, DirectShow will insert a splitter and then
// start trying codecs, both audio and video, video codecs first. If
// your sample grabber's set to connect to audio, unless we do this, it
// will try all the video codecs first. Connection times are sped up x10
// for audio with just this minor modification!
//----------------------------------------------------------------------------

class CSampleGrabberInPin : public CTransInPlaceInputPin
{
    friend class CSampleGrabberAllocator;
    friend class CSampleGrabber;

    CSampleGrabberAllocator * m_pPrivateAllocator;
    ALLOCATOR_PROPERTIES m_allocprops;
    BYTE * m_pBuffer;
    BOOL m_bMediaTypeChanged;

protected:

    CSampleGrabber * SampleGrabber( ) { return (CSampleGrabber*) m_pFilter; }
    HRESULT SetDeliveryBuffer( ALLOCATOR_PROPERTIES props, BYTE * m_pBuffer );

public:

    CSampleGrabberInPin( CTransInPlaceFilter * pFilter, HRESULT * pHr ) 
        : CTransInPlaceInputPin( TEXT("SampleGrabberInputPin"), pFilter, pHr, L"Input" )
        , m_pPrivateAllocator( NULL )
        , m_pBuffer( NULL )
        , m_bMediaTypeChanged( FALSE )
    {
        memset( &m_allocprops, 0, sizeof( m_allocprops ) );
    }

    ~CSampleGrabberInPin( )
    {
        if( m_pPrivateAllocator ) delete m_pPrivateAllocator;
    }

    // override to provide major media type for fast connects

    HRESULT GetMediaType( int iPosition, CMediaType *pMediaType );

    // override this or GetMediaType is never called

    STDMETHODIMP EnumMediaTypes( IEnumMediaTypes **ppEnum );

    // override this to refuse any allocators besides
    // the one the user wants, if this is set

    STDMETHODIMP NotifyAllocator( IMemAllocator *pAllocator, BOOL bReadOnly );

    // override this so we always return the special allocator, if necessary

    STDMETHODIMP GetAllocator( IMemAllocator **ppAllocator );

    HRESULT SetMediaType( const CMediaType *pmt );

};

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

class CSampleGrabber : public CTransInPlaceFilter
{
    friend class CSampleGrabberInPin;
    friend class CSampleGrabberAllocator;

protected:
	void* m_pUser;
    CMediaType m_mtAccept;
    SAMPLECALLBACK m_callback;
    CCritSec m_Lock; // serialize access to our data

    BOOL IsReadOnly( ) { return !m_bModifiesData; }

    // PURE, override this to ensure we get 
    // connected with the right media type
    HRESULT CheckInputType( const CMediaType * pmt );

    // PURE, override this to callback 
    // the user when a sample is received
    HRESULT Transform( IMediaSample * pms );

    // override this so we can return S_FALSE directly. 
    // The base class CTransInPlace
    // Transform( ) method is called by it's 
    // Receive( ) method. There is no way
    // to get Transform( ) to return an S_FALSE value 
    // (which means "stop giving me data"),
    // to Receive( ) and get Receive( ) to return S_FALSE as well.

    HRESULT Receive( IMediaSample * pms );

public:

    static CUnknown *WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

    // Expose ISampleGrabber
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
    DECLARE_IUNKNOWN;

    CSampleGrabber( IUnknown * pOuter, HRESULT * pHr, BOOL ModifiesData );

    // IGrabberSample
    HRESULT SetAcceptedMediaType( const CMediaType * pmt );
    HRESULT GetConnectedMediaType( CMediaType * pmt );
    HRESULT SetCallback( SAMPLECALLBACK Callback, void* pUser );
    HRESULT SetDeliveryBuffer( ALLOCATOR_PROPERTIES props, BYTE * m_pBuffer );
};


