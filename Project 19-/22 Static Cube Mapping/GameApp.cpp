#include "GameApp.h"
#include "XUtil.h"
#include "DXTrace.h"
#include "ModelManager.h"
#include "RenderStates.h"
#include "Camera.h"
#include "Texture2D.h"
#include "WinMin.h"
#include "LightHelper.h"
using namespace DirectX;

GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight)
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
    if (!D3DApp::Init())
        return false;

    m_TextureManager.Init(m_pd3dDevice.Get());
    m_ModelManager.Init(m_pd3dDevice.Get());

    // 务必先初始化所有渲染状态，以供下面的特效使用
    RenderStates::InitAll(m_pd3dDevice.Get());

    if (!m_BasicEffect.InitAll(m_pd3dDevice.Get()))
        return false;

    if (!m_SkyboxEffect.InitAll(m_pd3dDevice.Get()))
        return false;

    if (!InitResource())
        return false;

    return true;
}

void GameApp::OnResize()
{

    D3DApp::OnResize();
    
    m_pDepthTexture = std::make_unique<Depth2D>(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight);
    m_pDepthTexture->SetDebugObjectName("DepthTexture");

    // 摄像机变更显示
    if (m_pCamera != nullptr)
    {
        m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
        m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
        m_BasicEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
        m_SkyboxEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
    }
}

void GameApp::UpdateScene(float dt)
{
    m_CameraController.Update(dt);
    

    // 更新观察矩阵
    m_BasicEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
    m_BasicEffect.SetEyePos(m_pCamera->GetPosition());

    m_SkyboxEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
    
    
    
    
    
    
    ImVec2 mousePos = ImGui::GetMousePos();
    mousePos.x = std::clamp(mousePos.x, 0.0f, m_ClientWidth - 1.0f);
    mousePos.y = std::clamp(mousePos.y, 0.0f, m_ClientHeight - 1.0f);
    Ray ray = Ray::ScreenToRay(*m_pCamera, mousePos.x, mousePos.y);

    bool hitObject = false;
    mouseAtGround = false;
    std::string pickedObjStr = "None";
    XMFLOAT3 coordinate(0.0f,0.0f,0.0f);
 
    if (ray.Hit(m_BoundingSphere))
    {
        pickedObjStr = "Sphere";
        hitObject = true;
    }
    else if (ray.Hit(m_Ground.GetBoundingBox()))
    {
        pickedObjStr = "Ground";
        hitObject = true;
        mouseAtGround = true;
        DirectX::XMFLOAT3 boxMin;
        DirectX::XMFLOAT3 boxMax;

        boxMin.x = m_Ground.GetBoundingBox().Center.x - m_Ground.GetBoundingBox().Extents.x;
        boxMin.y = m_Ground.GetBoundingBox().Center.y - m_Ground.GetBoundingBox().Extents.y;
        boxMin.z = m_Ground.GetBoundingBox().Center.z - m_Ground.GetBoundingBox().Extents.z;
        boxMax.x = m_Ground.GetBoundingBox().Center.x + m_Ground.GetBoundingBox().Extents.x;
        boxMax.y = m_Ground.GetBoundingBox().Center.y + m_Ground.GetBoundingBox().Extents.y;
        boxMax.z = m_Ground.GetBoundingBox().Center.z + m_Ground.GetBoundingBox().Extents.z;

        // 计算每个方向的交点参数
        float t1 = (boxMin.x - ray.origin.x) / ray.direction.x;
        float t2 = (boxMax.x - ray.origin.x) / ray.direction.x;
        float t3 = (boxMin.y - ray.origin.y) / ray.direction.y;
        float t4 = (boxMax.y - ray.origin.y) / ray.direction.y;
        float t5 = (boxMin.z - ray.origin.z) / ray.direction.z;
        float t6 = (boxMax.z - ray.origin.z) / ray.direction.z;

        // 确定进入和离开的参数值
        float tMin = std::min(t1, t2);
        float tMax = std::max(t1, t2);
        tMin = std::max(tMin, std::min(t3, t4));
        tMax = std::min(tMax, std::max(t3, t4));
        tMin = std::max(tMin, std::min(t5, t6));
        tMax = std::min(tMax, std::max(t5, t6));

        // 判断是否相交
        if (tMin <= tMax) {
            // 计算进入点坐标
            coordinate.x = ray.origin.x + tMin * ray.direction.x;
            coordinate.y = ray.origin.y + tMin * ray.direction.y;
            coordinate.z = ray.origin.z + tMin * ray.direction.z;
        }
        

    }
    if (ImGui::Begin("Static Cube Mapping"))
    {
        ImGui::Text("Current Object: %s", pickedObjStr.c_str());
        ImGui::Text("Current Coord: %.3f,%.3f,%.3f", coordinate.x,coordinate.y,coordinate.z);
        
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            if (mouseAtGround)
            {
                CubeExist = true;
                m_Cube.SetVisible(1);
                m_Cube.GetTransform().SetPosition(coordinate.x, coordinate.y, coordinate.z);
            }
        }
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            if (ray.Hit(m_Cube.GetBoundingBox()))
            {
               CubeExist = false;
               m_Cube.SetVisible(0);
            }
        }
        static int skybox_item = 0;
        static const char* skybox_strs[] = {
            "Daylight",
            "Sunset",
            "Desert"
        };
        if (ImGui::Combo("Skybox", &skybox_item, skybox_strs, ARRAYSIZE(skybox_strs)))
        {

            Model* pModel = m_ModelManager.GetModel("Skybox");
            switch (skybox_item)
            {
            case 0: 
                m_BasicEffect.SetTextureCube(m_TextureManager.GetTexture("Daylight"));
                pModel->materials[0].Set<std::string>("$Skybox", "Daylight");
                break;
            case 1: 
                m_BasicEffect.SetTextureCube(m_TextureManager.GetTexture("Sunset"));
                pModel->materials[0].Set<std::string>("$Skybox", "Sunset");
                break;
            case 2: 
                m_BasicEffect.SetTextureCube(m_TextureManager.GetTexture("Desert")); 
                pModel->materials[0].Set<std::string>("$Skybox", "Desert");
                break;
            }
        }
    }
    ImGui::End();
    ImGui::Render();
}

void GameApp::DrawScene()
{
    // 创建后备缓冲区的渲染目标视图
    if (m_FrameCount < m_BackBufferCount)
    {
        ComPtr<ID3D11Texture2D> pBackBuffer;
        m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()));
        CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
        m_pd3dDevice->CreateRenderTargetView(pBackBuffer.Get(), &rtvDesc, m_pRenderTargetViews[m_FrameCount].ReleaseAndGetAddressOf());
    }

    float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    m_pd3dImmediateContext->ClearRenderTargetView(GetBackBufferRTV(), black);
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthTexture->GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    ID3D11RenderTargetView* pRTVs[1] = { GetBackBufferRTV() };
    m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, m_pDepthTexture->GetDepthStencil());
    D3D11_VIEWPORT viewport = m_pCamera->GetViewPort();
    m_pd3dImmediateContext->RSSetViewports(1, &viewport);

    // 绘制模型
    m_BasicEffect.SetRenderDefault();
    m_BasicEffect.SetReflectionEnabled(true);
    m_Sphere.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    if (CubeExist)
    {
        m_Cube.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    }
    m_BasicEffect.SetReflectionEnabled(false);
    m_Ground.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    m_Cylinder.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    

    // 绘制天空盒
    m_SkyboxEffect.SetRenderDefault();
    m_Skybox.Draw(m_pd3dImmediateContext.Get(), m_SkyboxEffect);


    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    HR(m_pSwapChain->Present(0, m_IsDxgiFlipModel ? DXGI_PRESENT_ALLOW_TEARING : 0));
}

bool GameApp::InitResource()
{
    // ******************
    // 初始化天空盒相关
    
    ComPtr<ID3D11Texture2D> pTex;
    D3D11_TEXTURE2D_DESC texDesc;
    std::string filenameStr;
    std::vector<ID3D11ShaderResourceView*> pCubeTextures;
    std::unique_ptr<TextureCube> pTexCube;
    // Daylight
    {
        filenameStr = "Texture\\evening_0.png";
        for (size_t i = 0; i < 6; ++i)
        {
            filenameStr[16] = '0' + (char)i;
            pCubeTextures.push_back(m_TextureManager.CreateFromFile(filenameStr));
        }

        pCubeTextures[0]->GetResource(reinterpret_cast<ID3D11Resource**>(pTex.ReleaseAndGetAddressOf()));
        pTex->GetDesc(&texDesc);
        pTexCube = std::make_unique<TextureCube>(m_pd3dDevice.Get(), texDesc.Width, texDesc.Height, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
        pTexCube->SetDebugObjectName("Daylight");
        for (uint32_t i = 0; i < 6; ++i)
        {
            pCubeTextures[i]->GetResource(reinterpret_cast<ID3D11Resource**>(pTex.ReleaseAndGetAddressOf()));
            m_pd3dImmediateContext->CopySubresourceRegion(pTexCube->GetTexture(), D3D11CalcSubresource(0, i, 1), 0, 0, 0, pTex.Get(), 0, nullptr);
        }
        m_TextureManager.AddTexture("Daylight", pTexCube->GetShaderResource());
    }
    
    // Sunset
    {
        filenameStr = "Texture\\sunset0.bmp";
        pCubeTextures.clear();
        for (size_t i = 0; i < 6; ++i)
        {
            filenameStr[14] = '0'+(char)i;
            pCubeTextures.push_back(m_TextureManager.CreateFromFile(filenameStr));
        }
        pCubeTextures[0]->GetResource(reinterpret_cast<ID3D11Resource**>(pTex.ReleaseAndGetAddressOf()));
        pTex->GetDesc(&texDesc);
        pTexCube = std::make_unique<TextureCube>(m_pd3dDevice.Get(), texDesc.Width, texDesc.Height, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
        pTexCube->SetDebugObjectName("Sunset");
        for (uint32_t i = 0; i < 6; ++i)
        {
            pCubeTextures[i]->GetResource(reinterpret_cast<ID3D11Resource**>(pTex.ReleaseAndGetAddressOf()));
            m_pd3dImmediateContext->CopySubresourceRegion(pTexCube->GetTexture(), D3D11CalcSubresource(0, i, 1), 0, 0, 0, pTex.Get(), 0, nullptr);
        }
        m_TextureManager.AddTexture("Sunset", pTexCube->GetShaderResource());
    }
    
    // Desert
    m_TextureManager.AddTexture("Desert", m_TextureManager.CreateFromFile("Texture\\desertcube1024.dds", false, true));

    m_BasicEffect.SetTextureCube(m_TextureManager.GetTexture("Daylight"));
    
    // ******************
    // 初始化游戏对象
    //
    //cube
    {
        Model* pModel;
        pModel = m_ModelManager.CreateFromGeometry("Cube", Geometry::CreateBox());
        pModel->SetDebugObjectName("Cube");
      m_TextureManager.CreateFromFile("Texture\\water2.dds");
        pModel->materials[0].Set<std::string>("$Diffuse", "Texture\\water2.dds");
       pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
       pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
        pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
        pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
        pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
        m_Cube.SetModel(pModel);
       
        
    }
    // 球体
    {
        Model* pModel = m_ModelManager.CreateFromGeometry("Sphere", Geometry::CreateSphere());
        pModel->SetDebugObjectName("Sphere");
        m_TextureManager.CreateFromFile("Texture\\stone.dds");
        pModel->materials[0].Set<std::string>("$Diffuse", "Texture\\stone.dds");
        pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
        pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f));
        pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));
        pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
        pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));
        
        m_Sphere.SetModel(pModel);
        m_Sphere.GetTransform().SetPosition(0.0f, 0.0f, 0.0f);
        
    }
    // 地面
    {
        Model* pModel = m_ModelManager.CreateFromGeometry("Ground", Geometry::CreateBox(1, 1, 1));
        pModel->SetDebugObjectName("Ground");
        m_TextureManager.CreateFromFile("Texture\\floor.dds");
        pModel->materials[0].Set<std::string>("$Diffuse", "Texture\\floor.dds");
        pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
        pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.3f, 0.0f, 0.4f, 0.1f));
        pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
        pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
        pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
        m_Ground.SetModel(pModel);
        
       m_Ground.GetTransform().SetPosition(0.0f, -3.0f, 0.0f);
       m_Ground.GetTransform().SetScale(100, 1, 100);
    
    }	
    // 柱体
    {
        Model* pModel = m_ModelManager.CreateFromGeometry("Cylinder", Geometry::CreateCylinder(0.5f, 2.0f));
        pModel->SetDebugObjectName("Cylinder");
        m_TextureManager.CreateFromFile("Texture\\bricks.dds");
        pModel->materials[0].Set<std::string>("$Diffuse", "Texture\\bricks.dds");
        pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
        pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.1f, 0.1f, 0.6f, 1.0f));
        pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
        pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
        pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
        m_Cylinder.SetModel(pModel);
        m_Cylinder.GetTransform().SetPosition(0.0f, -1.99f, 0.0f);
    }
    // 天空盒立方体
    Model* pModel = m_ModelManager.CreateFromGeometry("Skybox", Geometry::CreateBox());
    pModel->SetDebugObjectName("Skybox");
    pModel->materials[0].Set<std::string>("$Skybox", "Daylight");
    m_Skybox.SetModel(pModel);
    // ******************
    // 初始化摄像机
    //
    auto camera = std::make_shared<FirstPersonCamera>();
    m_pCamera = camera;
    m_CameraController.InitCamera(camera.get());
    camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
    camera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
    camera->LookTo(XMFLOAT3(0.0f, 0.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

    m_BasicEffect.SetViewMatrix(camera->GetViewMatrixXM());
    m_BasicEffect.SetProjMatrix(camera->GetProjMatrixXM());
    m_SkyboxEffect.SetViewMatrix(camera->GetViewMatrixXM());
    m_SkyboxEffect.SetProjMatrix(camera->GetProjMatrixXM());



    // ******************
    // 初始化不会变化的值
    //

    // 方向光
    DirectionalLight dirLight[4]{};
    dirLight[0].ambient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);
    dirLight[0].diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    dirLight[0].specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
    dirLight[0].direction = XMFLOAT3(-0.577f, -0.577f, 0.577f);
    dirLight[1] = dirLight[0];
    dirLight[1].direction = XMFLOAT3(0.577f, -0.577f, 0.577f);
    dirLight[2] = dirLight[0];
    dirLight[2].direction = XMFLOAT3(0.577f, -0.577f, -0.577f);
    dirLight[3] = dirLight[0];
    dirLight[3].direction = XMFLOAT3(-0.577f, -0.577f, -0.577f);
    for (int i = 0; i < 4; ++i)
        m_BasicEffect.SetDirLight(i, dirLight[i]);


    return true;
}

