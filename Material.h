#pragma once

#include <DirectXMath.h>

#include <map>
#include <memory>

class Texture;

//clang-format off
struct alignas(16) MaterialProperties
{
    //The material properties must be aligned to a 16-byte boundary
    //To gurantee alignment, the material properties structure will be allocated in aligned memory
    MaterialProperties(DirectX::XMFLOAT4 diffuse = { 1.0f, 1.0f, 1.0f, 1.0f },
        DirectX::XMFLOAT4 specular = { 1.0f, 1.0f, 1.0f, 1.0f },
        float specularPower = 128.0f,
        DirectX::XMFLOAT4 ambient = { 0.1f, 0.1f, 0.1f, 1.0f },
        DirectX::XMFLOAT4 emissive = { 0.0f, 0.0f, 0.0f, 1.0f },
        DirectX::XMFLOAT4 reflectance = { 0.0f, 0.0f, 0.0f, 0.0f }, float opacity = 1.0f,
        float indexOfRefraction = 0.0f, float bumpIntensity = 1.0f,
        float alphaThreshold = 0.1f)
        : Diffuse(diffuse)
        , Specular(specular)
        , Emissive(emissive)
        , Ambient(ambient)
        , Reflectance(reflectance)
        , Opacity(opacity)
        , SpecularPower(specularPower)
        , IndexOfRefraction(indexOfRefraction)
        , BumpIntensity(bumpIntensity)
        , HasAmbientTexture(false)
        , HasEmissiveTexture(false)
        , HasDiffuseTexture(false)
        , HasSpecularTexture(false)
        , HasSpecularPowerTexture(false)
        , HasNormalTexture(false)
        , HasBumpTexture(false)
        , HasOpacityTexture(false)
    {}
    
    DirectX::XMFLOAT4 Diffuse;
    //------------------------------(16 byte boundary)
    DirectX::XMFLOAT4 Specular;
    //--------------------------------(16 byte boundary)
    DirectX::XMFLOAT4 Emissive;
    //--------------------------------(16 byte boundary)
    DirectX::XMFLOAT4 Ambient;
    //--------------------------------(16 byte boundary)
    DirectX::XMFLOAT4 Reflectance;
    //--------------------------------(16 byte boundary)
    float Opacity; //If Opacity < 1, then the material is transparent
    float SpecularPower;
    float IndexOfRefraction; //For transparent materials, IOR > 0
    float BumpIntensity; //When using bump textures/height maps, we need to scale the height values so the normals are visible
    //-------------------------------(16 byte boundary)
    uint32_t HasAmbientTexture;
    uint32_t HasEmissiveTexture;
    uint32_t HasDiffuseTexture;
    uint32_t HasSpecularTexture;
    //------------------------------(16 byte boundary)
    uint32_t HasSpecularPowerTexture;
    uint32_t HasNormalTexture;
    uint32_t HasBumpTexture;
    uint32_t HasOpacityTexture;
    //-------------------------------(16 byte boundary)
    //Total of 128 bytes
};
//clang format on

class Material
{
public:
    //These are the texture slots that can be bound to the material
    enum class TextureType
    {
        Ambient,
        Emissive,
        Diffuse,
        Specular,
        SpecularPower,
        Normal,
        Bump,
        Opacity,
        NumTypes,
    };

    Material(const MaterialProperties& materialProperties = MaterialProperties());
    Material(const Material& copy);

    ~Material() = default;

    const DirectX::XMFLOAT4& GetAmbientColor() const;
    void SetAmbientColor(const DirectX::XMFLOAT4& ambient);

    const DirectX::XMFLOAT4& GetDiffuseColor() const;
    void SetDiffuseColor(const DirectX::XMFLOAT4& diffuse);

    const DirectX::XMFLOAT4& GetEmissiveColor() const;
    void SetEmissiveColor(const DirectX::XMFLOAT4& emissive);

    const DirectX::XMFLOAT4& GetSpecularColor() const;
    void SetSpecularColor(const DirectX::XMFLOAT4& specular);

    float GetSpecularPower() const;
    void SetSpecularPower(float specularPower);

    const DirectX::XMFLOAT4& GetReflectance() const;
    void SetReflectance(const DirectX::XMFLOAT4& reflectance);

    const float GetOpacity() const;
    void SetOpacity(float opacity);

    float GetIndexOfRefraction() const;
    void SetIndexOfRefraction(float indexOfRefraction);

    //When using bump maps, we can adjust the intensity of the normals generated from the bump maps, or inverse the normals by using a negative intensity
    // Default bump intensity is 1.0 and a value of 0 will remove the bump effect completely
    float GetBumpIntensity() const;
    void SetBumpIntensity(float bumpIntensity);

    std::shared_ptr<Texture> GetTexture(TextureType ID) const;
    void SetTexture(TextureType type, std::shared_ptr<Texture> texture);

    //This material defines a transparent material. If the opacity is < 1, or there is an opacity map, or the diffuse texture has an alpha channel
    bool IsTransparent() const;

    const MaterialProperties& GetMaterialProperties() const;
    void SetMaterialProperties(const MaterialProperties& materialProperties);

	//Define some materials
	static const Material Red;
    static const Material Green;
    static const Material Blue;
    static const Material Cyan;
    static const Material Magenta;
    static const Material Yellow;
    static const Material White;
    static const Material Black;
    static const Material Emerald;
    static const Material Jade;
    static const Material Obsidian;
    static const Material Pearl;
    static const Material Ruby;
    static const Material Turquoise;
    static const Material Brass;
    static const Material Bronze;
    static const Material Chrome;
    static const Material Copper;
    static const Material Gold;
    static const Material Silver;
    static const Material BlackPlastic;
    static const Material CyanPlastic;
    static const Material GreenPlastic;
    static const Material RedPlastic;
    static const Material WhitePlastic;
    static const Material YellowPlastic;
    static const Material BlackRubber;
    static const Material CyanRubber;
    static const Material GreenRubber;
    static const Material RedRubber;
    static const Material WhiteRubber;
    static const Material YellowRubber;

protected:
private:
    using TextureMap = std::map<TextureType, std::shared_ptr<Texture>>;
    //A unique pointer with a custom allocator/deallocator to ensure alignment
    using MaterialPropertiesPtr = std::unique_ptr<MaterialProperties, void(*)(MaterialProperties*)>;

    MaterialPropertiesPtr m_MaterialProperties;
    TextureMap m_Textures;
};