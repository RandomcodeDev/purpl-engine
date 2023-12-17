#include <comdef.h>

#include "purpl/purpl.h"

BEGIN_EXTERN_C
#define RENDERSYSTEM_IMPLEMENTATION
#include "engine/engine.h"
END_EXTERN_C

#define HRESULT_CHECK(Call, ...) \
    do \
    { \
        HRESULT Result_ = (Call); \
        _com_error Error_(Result_); \
        if ( Result_ != ERROR_SUCCESS __VA_ARGS__ ) \
        { \
            CommonError("DirectX call " #Call " failed: %s (HRESULT 0x%08X)", Error_.ErrorMessage(), Result_); \
        } \
    } while (0)

//
// Model data
//

typedef struct DIRECTX9_MODEL_DATA
{
    IDirect3DVertexBuffer9* VertexBuffer;
    IDirect3DIndexBuffer9* IndexBuffer;
} DIRECTX9_MODEL_DATA, *PDIRECTX9_MODEL_DATA;

//
// Core objects
//

static IDirect3D9* Direct3d;
static IDirect3DDevice9* Device;

//
// Vertex stuff
//

IDirect3DVertexDeclaration9* VertexDeclaration;

static
VOID
CreateDevice(
    VOID
    )
{
    LogDebug("Creating IDirect3DDevice9");

    D3DPRESENT_PARAMETERS PresentParameters = {0};
    PresentParameters.hDeviceWindow = (HWND)PlatformGetVideoObject();
    PresentParameters.Windowed = TRUE;
    PresentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    //PresentParameters.BackBufferFormat = D3DFMT_X8R8G8B8;
    //PlatformGetVideoSize(
    //    &PresentParameters.BackBufferWidth,
    //    &PresentParameters.BackBufferHeight
    //    );
    PresentParameters.EnableAutoDepthStencil = TRUE;
    PresentParameters.AutoDepthStencilFormat = D3DFMT_D16;

    Device = nullptr;
    HRESULT_CHECK(Direct3d->CreateDevice(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        (HWND)PlatformGetVideoObject(),
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &PresentParameters,
        &Device
        ));
}

static
VOID
CreateVertexDeclaration(
    VOID
    )
{
    LogDebug("Creating IDirect3DVertexDeclaration9");

    const D3DVERTEXELEMENT9 VertexElements[] = {
        {
            0,
            offsetof(VERTEX, Position),
            D3DDECLTYPE_FLOAT3,
            D3DDECLMETHOD_DEFAULT,
            D3DDECLUSAGE_POSITION,
            0
        },
        {
            0,
            offsetof(VERTEX, Colour),
            D3DDECLTYPE_FLOAT4,
            D3DDECLMETHOD_DEFAULT,
            D3DDECLUSAGE_COLOR,
            0
        },
        {
            0,
            offsetof(VERTEX, TextureCoordinate),
            D3DDECLTYPE_FLOAT2,
            D3DDECLMETHOD_DEFAULT,
            D3DDECLUSAGE_TEXCOORD,
            0
        },
        {
            0,
            offsetof(VERTEX, Normal),
            D3DDECLTYPE_FLOAT3,
            D3DDECLMETHOD_DEFAULT,
            D3DDECLUSAGE_NORMAL,
            0
        },
        D3DDECL_END()
    };

    VertexDeclaration = nullptr;
    HRESULT_CHECK(Device->CreateVertexDeclaration(
        VertexElements,
        &VertexDeclaration
        ));
}

static
BOOL
CheckResized(
    VOID
    )
{
    IDirect3DSwapChain9* Swapchain;
    UINT32 Width;
    UINT32 Height;

    Device->GetSwapChain(
        0,
        &Swapchain
        );

    D3DPRESENT_PARAMETERS PresentParameters = {0};
    Swapchain->GetPresentParameters(&PresentParameters);
    Width = 0;
    Height = 0;
    PlatformGetVideoSize(
        &Width,
        &Height
        );

    return Width != PresentParameters.BackBufferWidth || Height != PresentParameters.BackBufferHeight;
}

static
VOID
Resize(
    VOID
    )
{
    if ( VertexDeclaration )
    {
        VertexDeclaration->Release();
    }
    if ( Device )
    {
        Device->Release();
    }
    CreateDevice();
    CreateVertexDeclaration();

    D3DLIGHT9 Light = {0};
    Light.Type = D3DLIGHT_POINT;
    Light.Diffuse.r = 0.7f;
    Light.Diffuse.g = 0.7f;
    Light.Diffuse.b = 0.7f;
    Light.Diffuse.a = 0.7f;
    Light.Position.x = 0.0f;
    Light.Position.y = 0.0f;
    Light.Position.z = 0.0f;
    Light.Attenuation1 = 1.0f;
    Light.Range = 100.0f;

    Device->SetLight(
        0,
        &Light
        );
    Device->LightEnable(
        0,
        TRUE
        );

    D3DMATERIAL9 Material = {0};
    Material.Diffuse.r = 1.0f;
    Material.Diffuse.g = 1.0f;
    Material.Diffuse.b = 1.0f;
    Material.Diffuse.a = 1.0f;
    Material.Ambient.r = 1.0f;
    Material.Ambient.g = 1.0f;
    Material.Ambient.b = 1.0f;
    Material.Ambient.a = 1.0f;
    Device->SetMaterial(&Material);
}

EXTERN_C
VOID
DirectX9Initialize(
    VOID
    )
/*++

Routine Description:

    Creates objects needed for rendering.

Arguments:

    None.

Return Value:

    None.

--*/
{
    LogDebug("Initializing DirectX 9");

//#ifdef PURPL_DEBUG
//    LogDebug("Creating IDirect3D9 with Direct3DCreate9On12");
//    Direct3d = Direct3DCreate9On12(
//        D3D_SDK_VERSION,
//        NULL,
//        0
//        );
//#else
    LogDebug("Creating IDirect3D9 with Direct3DCreate9");
    Direct3d = Direct3DCreate9(D3D_SDK_VERSION);
//#endif
    if ( !Direct3d )
    {
        CommonError("Failed to create IDirect3D9");
    }

    Resize();

    LogDebug("DirectX 9 initialization succeeded");
}

EXTERN_C
VOID
DirectX9BeginCommands(
    _In_ PRENDER_GLOBAL_UNIFORM_DATA UniformData
    )
/*++

Routine Description:

    Prepares to render a frame.

Arguments:

    UniformData - Global uniform data.

Return Value:

    None.

--*/
{
    if ( CheckResized() )
    {
        Resize();
        return;
    }

    Device->Clear(
        0,
        nullptr,
        D3DCLEAR_TARGET,
        D3DCOLOR_XRGB(0, 0, 0),
        1.0f,
        0
        );
    Device->Clear(
        0,
        nullptr,
        D3DCLEAR_ZBUFFER,
        D3DCOLOR_XRGB(0, 0, 0),
        1.0f,
        0
        );

    Device->BeginScene();

    Device->SetVertexDeclaration(VertexDeclaration);

    Device->SetRenderState(
        D3DRS_LIGHTING,
        TRUE
        );
    Device->SetRenderState(
        D3DRS_CULLMODE,
        D3DCULL_NONE
        );
    Device->SetRenderState(
        D3DRS_CLIPPING,
        FALSE
        );
    Device->SetRenderState(
        D3DRS_ZENABLE,
        TRUE
        );

    Device->SetRenderState(
        D3DRS_AMBIENT,
        D3DCOLOR_XRGB(50, 50, 50)
        );

    Device->SetTransform(
        D3DTS_VIEW,
        (D3DMATRIX*)&UniformData->ViewTransform
        );
    Device->SetTransform(
        D3DTS_PROJECTION,
        (D3DMATRIX*)&UniformData->ProjectionTransform
        );
}

EXTERN_C
VOID
DirectX9DrawModel(
    _In_ PMODEL Model,
    _In_ PRENDER_MODEL_UNIFORM_DATA UniformData
    )
/*++

Routine Description:

    Draws a model.

Arguments:

    Model - The model to draw.

    UniformData - Object specific uniform data.

Return Value:

    None.

--*/
{
    PDIRECTX9_MODEL_DATA ModelData = (PDIRECTX9_MODEL_DATA)Model->Handle;
    Device->SetStreamSource(
        0,
        ModelData->VertexBuffer,
        0,
        sizeof(VERTEX)
        );
    Device->SetTransform(
        D3DTS_WORLD,
        (D3DMATRIX*)&UniformData->ModelTransform
        );
    if ( Model->Mesh->IndexCount )
    {
        Device->SetIndices(ModelData->IndexBuffer);
        Device->DrawIndexedPrimitive(
            D3DPT_TRIANGLELIST,
            0,
            0,
            Model->Mesh->VertexCount,
            0,
            Model->Mesh->IndexCount
            );
    }
    else
    {
        Device->DrawPrimitive(
            D3DPT_TRIANGLELIST,
            0,
            Model->Mesh->VertexCount
            );
    }
}

EXTERN_C
VOID
DirectX9PresentFrame(
    VOID
    )
/*++

Routine Description:

    Presents a frame.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Device->EndScene();

    Device->Present(
        nullptr,
        nullptr,
        nullptr,
        nullptr
        );
}

EXTERN_C
VOID
DirectX9Shutdown(
    VOID
    )
/*++

Routine Description:

    Destroys objetcs.

Arguments:

    None.

Return Value:

    None.

--*/
{
    LogDebug("Shutting down DirectX 9");

    if ( VertexDeclaration )
    {
        LogDebug("Destroying IDirect3DVertexDeclaration9");
        VertexDeclaration->Release();
    }

    if ( Device )
    {
        LogDebug("Destroying IDirect3DDevice9");
        Device->Release();
    }

    if ( Direct3d )
    {
        LogDebug("Destroying IDirect3D9");
        Direct3d->Release();
    }

    LogDebug("DirectX 9 shutdown succeeded");
}

EXTERN_C
VOID
DirectX9CreateShader(
    _In_ PSHADER SourceShader
    )
/*++

Routine Description:

Arguments:

Return Value:

--*/
{

}

EXTERN_C
VOID
DirectX9DestroyShader(
    _In_ PSHADER Shader
    )
/*++

Routine Description:

Arguments:

Return Value:

--*/
{

}

EXTERN_C
VOID
DirectX9UseMesh(
    _In_ PMODEL SourceModel
    )
/*++

Routine Description:

    Sets up a mesh.

Arguments:

    SourceModel - The mesh to set up.

Return Value:

    None.

--*/
{
    PDIRECTX9_MODEL_DATA ModelData;
    PVOID Address;
    SIZE_T Size;

    ModelData = (PDIRECTX9_MODEL_DATA)PURPL_ALLOC(
        1,
        sizeof(DIRECTX9_MODEL_DATA)
        );
    if ( !ModelData )
    {
        CommonError("Failed to allocate memory for DirectX 9 data for model %s", SourceModel->Name);
    }
    SourceModel->Handle = ModelData;

    Size = SourceModel->Mesh->VertexCount * sizeof(VERTEX);
    HRESULT_CHECK(Device->CreateVertexBuffer(
        Size,
        0,
        0,
        D3DPOOL_MANAGED,
        &ModelData->VertexBuffer,
        nullptr
        ));
    Address = nullptr;
    HRESULT_CHECK(ModelData->VertexBuffer->Lock(
        0,
        Size,
        &Address,
        0
        ));
    memcpy(
        Address,
        SourceModel->Mesh->Vertices,
        Size
        );
    HRESULT_CHECK(ModelData->VertexBuffer->Unlock());

    if ( SourceModel->Mesh->IndexCount )
    {
        Size = SourceModel->Mesh->IndexCount * sizeof(ivec3);
        HRESULT_CHECK(Device->CreateIndexBuffer(
            Size,
            0,
            D3DFMT_INDEX32,
            D3DPOOL_MANAGED,
            &ModelData->IndexBuffer,
            nullptr
            ));
        Address = nullptr;
        HRESULT_CHECK(ModelData->IndexBuffer->Lock(
            0,
            Size,
            &Address,
            0
            ));
        memcpy(
            Address,
            SourceModel->Mesh->Indices,
            Size
            );
        HRESULT_CHECK(ModelData->IndexBuffer->Unlock());
    }
}

EXTERN_C
VOID
DirectX9DestroyModel(
    _In_ PMODEL Model
    )
/*++

Routine Description:

    Cleans up DirectX 9 model data.

Arguments:

    Model - The model to clean up the data of.

Return Value:

    None.

--*/
{
    PDIRECTX9_MODEL_DATA ModelData;

    ModelData = (PDIRECTX9_MODEL_DATA)Model->Handle;
    ModelData->VertexBuffer->Release();
    ModelData->IndexBuffer->Release();
}

EXTERN_C
VOID
DirectX9UseTexture(
    _In_ PRENDER_TEXTURE Texture
    )
/*++

Routine Description:

Arguments:

Return Value:

--*/
{

}

EXTERN_C
VOID
DirectX9DestroyTexture(
    _In_ PRENDER_TEXTURE Texture
    )
/*++

Routine Description:

Arguments:

Return Value:

--*/
{

}
