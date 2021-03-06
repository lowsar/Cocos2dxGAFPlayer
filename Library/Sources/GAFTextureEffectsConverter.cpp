#include "GAFPrecompiled.h"
#include "GAFTextureEffectsConverter.h"
#include "GAFShaderManager.h"

#include "GAFFilterData.h"

const int kGaussianKernelSize = 9;

GAFTextureEffectsConverter::GAFTextureEffectsConverter()
{

}

GAFTextureEffectsConverter::~GAFTextureEffectsConverter()
{

}

static GAFTextureEffectsConverter * _sharedConverter = NULL;

GAFTextureEffectsConverter * GAFTextureEffectsConverter::sharedConverter()
{
    if (!_sharedConverter)
    {
        _sharedConverter = new GAFTextureEffectsConverter();
    }
    return _sharedConverter;
}

CCRenderTexture* GAFTextureEffectsConverter::dropShadowTextureFromTexture(CCTexture2D * aTexture, const CCRect& rect, GAFDropShadowFilterData* data)
{
    const float blurRadiusX = (data->blurSize.width / 4.f);
    const float blurRadiusY = (data->blurSize.height / 4.f);

    CCSize rTextureSize = CCSizeMake(rect.size.width + 2 * (kGaussianKernelSize / 2) * blurRadiusX,
        rect.size.height + 2 * (kGaussianKernelSize / 2) * blurRadiusY);

    CCGLProgram * shader = programForBlurShaderWithName("GaussianBlur", "Shaders/GlowVertexShader.vs", "Shaders/GlowFragmentShader.fs");

    CCRenderTexture *rTexture1 = CCRenderTexture::create(
        static_cast<int>(rTextureSize.width),
        static_cast<int>(rTextureSize.height));

    CCRenderTexture *rTexture2 = CCRenderTexture::create(
        static_cast<int>(rTextureSize.width),
        static_cast<int>(rTextureSize.height));

    if (!shader)
    {
        return NULL;
    }

    GLint texelWidthOffset = glGetUniformLocation(shader->getProgram(), "texelWidthOffset");
    GLint texelHeightOffset = glGetUniformLocation(shader->getProgram(), "texelHeightOffset");
    GLint glowColor = glGetUniformLocation(shader->getProgram(), "glowColor");

    CHECK_GL_ERROR_DEBUG();
    {
        CCSprite *sprite = CCSprite::createWithTexture(aTexture, rect);
        sprite->setPosition(CCPointMake(rTextureSize.width / 2, rTextureSize.height / 2));
        ccBlendFunc blend = { GL_ONE, GL_ZERO };
        sprite->setBlendFunc(blend);

        rTexture2->beginWithClear(0, 0, 0, 0);
        sprite->visit();
        rTexture2->end();
    }
    CHECK_GL_ERROR_DEBUG();
    {
        // Render rTexture2 to rTexture1 (horizontal)
        GLfloat texelWidthValue = blurRadiusX / (GLfloat)rTextureSize.width;
        GLfloat texelHeightValue = 0;

        rTexture2->getSprite()->setPosition(CCPointMake(rTextureSize.width / 2, rTextureSize.height / 2));
        rTexture2->getSprite()->setShaderProgram(shader);
        shader->use();

        glUniform1f(texelWidthOffset, texelWidthValue);
        glUniform1f(texelHeightOffset, texelHeightValue);

        glUniform4fv(glowColor, 1, &data->color.r);

        ccBlendFunc blend = { GL_ONE, GL_ONE_MINUS_SRC_ALPHA };
        rTexture2->getSprite()->setBlendFunc(blend);
        rTexture1->beginWithClear(0, 0, 0, 0);
        rTexture2->getSprite()->visit();
        rTexture1->end();
    }

    CHECK_GL_ERROR_DEBUG();
    {
        // Render rTexture1 to rTexture2 (vertical)
        GLfloat texelWidthValue = 0;
        GLfloat texelHeightValue = blurRadiusY / (GLfloat)rTextureSize.height;

        rTexture1->getSprite()->setPosition(CCPointMake(rTextureSize.width / 2, rTextureSize.height / 2));
        rTexture1->getSprite()->setShaderProgram(shader);
        shader->use();

        glUniform1f(texelWidthOffset, texelWidthValue);
        glUniform1f(texelHeightOffset, texelHeightValue);
        glUniform4fv(glowColor, 1, &data->color.r);

        ccBlendFunc blend = { GL_ONE, GL_ONE_MINUS_SRC_ALPHA };
        rTexture1->getSprite()->setBlendFunc(blend);
        rTexture2->beginWithClear(0, 0, 0, 0);
        rTexture1->getSprite()->visit();
        rTexture2->end();
    }

    return rTexture2;
}

CCRenderTexture * GAFTextureEffectsConverter::glowTextureFromTexture(CCTexture2D * aTexture, const CCRect& rect, GAFGlowFilterData* data)
{
    const float blurRadiusX = (data->blurSize.width / 4.f);
    const float blurRadiusY = (data->blurSize.height / 4.f);

    CCSize rTextureSize = CCSizeMake(rect.size.width + 2 * (kGaussianKernelSize / 2) * blurRadiusX,
        rect.size.height + 2 * (kGaussianKernelSize / 2) * blurRadiusY);

    CCRenderTexture *rTexture1 = CCRenderTexture::create(static_cast<int>(rTextureSize.width),
        static_cast<int>(rTextureSize.height));

    CCRenderTexture *rTexture2 = CCRenderTexture::create(static_cast<int>(rTextureSize.width),
        static_cast<int>(rTextureSize.height));

    CCGLProgram * shader = programForBlurShaderWithName("GaussianBlur", "Shaders/GlowVertexShader.vs", "Shaders/GlowFragmentShader.fs");

    if (!shader)
    {
        return NULL;
    }

    GLint texelWidthOffset = glGetUniformLocation(shader->getProgram(), "texelWidthOffset");
    GLint texelHeightOffset = glGetUniformLocation(shader->getProgram(), "texelHeightOffset");
    GLint glowColor = glGetUniformLocation(shader->getProgram(), "glowColor");
    
    CHECK_GL_ERROR_DEBUG();
    {
        CCSprite *sprite = CCSprite::createWithTexture(aTexture, rect);
        sprite->setPosition(CCPointMake(rTextureSize.width / 2, rTextureSize.height / 2));
        ccBlendFunc blend = { GL_ONE, GL_ZERO };
        sprite->setBlendFunc(blend);

        rTexture2->beginWithClear(0, 0, 0, 0);
        sprite->visit();
        rTexture2->end();
    }
    CHECK_GL_ERROR_DEBUG();
    {
        // Render rTexture2 to rTexture1 (horizontal)
        GLfloat texelWidthValue = blurRadiusX / (GLfloat)rTextureSize.width;
        GLfloat texelHeightValue = 0;

        rTexture2->getSprite()->setPosition(CCPointMake(rTextureSize.width / 2, rTextureSize.height / 2));
        rTexture2->getSprite()->setShaderProgram(shader);
        shader->use();

        glUniform1f(texelWidthOffset, texelWidthValue);
        glUniform1f(texelHeightOffset, texelHeightValue);

        glUniform4fv(glowColor, 1, &data->color.r);

        ccBlendFunc blend = { GL_ONE, GL_ZERO };
        rTexture2->getSprite()->setBlendFunc(blend);
        rTexture1->beginWithClear(0, 0, 0, 0);
        rTexture2->getSprite()->visit();
        rTexture1->end();
    }

    CHECK_GL_ERROR_DEBUG();
    {
        // Render rTexture1 to rTexture2 (vertical)
        GLfloat texelWidthValue = 0;
        GLfloat texelHeightValue = blurRadiusY / (GLfloat)rTextureSize.height;

        rTexture1->getSprite()->setPosition(CCPointMake(rTextureSize.width / 2, rTextureSize.height / 2));
        rTexture1->getSprite()->setShaderProgram(shader);
        shader->use();

        glUniform1f(texelWidthOffset, texelWidthValue);
        glUniform1f(texelHeightOffset, texelHeightValue);
        glUniform4fv(glowColor, 1, &data->color.r);

        ccBlendFunc blend = { GL_ONE, GL_ZERO };
        rTexture1->getSprite()->setBlendFunc(blend);
        rTexture2->beginWithClear(0, 0, 0, 0);
        rTexture1->getSprite()->visit();
        rTexture2->end();
    }
    CHECK_GL_ERROR_DEBUG();
    {
        CCSprite *sprite = CCSprite::createWithTexture(aTexture, rect);
        sprite->setPosition(CCPointMake(rTextureSize.width / 2, rTextureSize.height / 2));

        rTexture2->begin();
        sprite->visit();
        rTexture2->end();
    }
    CHECK_GL_ERROR_DEBUG();

    return rTexture2;
}

CCRenderTexture * GAFTextureEffectsConverter::gaussianBlurredTextureFromTexture(CCTexture2D * aTexture, const CCRect& rect, float aBlurRadiusX, float aBlurRadiusY)
{
    const float blurRadiusX = (aBlurRadiusX / 4.f);
    const float blurRadiusY = (aBlurRadiusY / 4.f);

    CCSize rTextureSize = CCSizeMake(rect.size.width + 2 * (kGaussianKernelSize / 2) * blurRadiusX,
        rect.size.height + 2 * (kGaussianKernelSize / 2) * blurRadiusY);

    CCRenderTexture *rTexture1 = CCRenderTexture::create(static_cast<int>(rTextureSize.width),
                                                         static_cast<int>(rTextureSize.height));

    CCRenderTexture *rTexture2 = CCRenderTexture::create(static_cast<int>(rTextureSize.width),
                                                         static_cast<int>(rTextureSize.height));

    CCGLProgram * shader = programForBlurShaderWithName("GaussianBlur", "Shaders/GaussianBlurVertexShader.vs", "Shaders/GaussianBlurFragmentShader.fs");
    if (!shader)
    {
        return NULL;
    }
    GLint texelWidthOffset = (GLint)glGetUniformLocation(shader->getProgram(), "texelWidthOffset");
    GLint texelHeightOffset = (GLint)glGetUniformLocation(shader->getProgram(), "texelHeightOffset");
    CHECK_GL_ERROR_DEBUG();
    {
        CCSprite *sprite = CCSprite::createWithTexture(aTexture, rect);
        sprite->setPosition(CCPointMake(rTextureSize.width / 2, rTextureSize.height / 2));
        ccBlendFunc blend = { GL_ONE, GL_ZERO };
        sprite->setBlendFunc(blend);

        rTexture2->beginWithClear(0, 0, 0, 0);
        sprite->visit();
        rTexture2->end();
    }
    CHECK_GL_ERROR_DEBUG();
    {
        // Render rTexture2 to rTexture1 (horizontal)
        GLfloat texelWidthValue = blurRadiusX / (GLfloat)rTextureSize.width;
        GLfloat texelHeightValue = 0;

        rTexture2->getSprite()->setPosition(CCPointMake(rTextureSize.width / 2, rTextureSize.height / 2));
        rTexture2->getSprite()->setShaderProgram(shader);
        shader->use();
        glUniform1f(texelWidthOffset, texelWidthValue);
        glUniform1f(texelHeightOffset, texelHeightValue);
        ccBlendFunc blend = { GL_ONE, GL_ZERO };
        rTexture2->getSprite()->setBlendFunc(blend);
        rTexture1->beginWithClear(0, 0, 0, 0);
        rTexture2->getSprite()->visit();
        rTexture1->end();
    }

    CHECK_GL_ERROR_DEBUG();
    {
        // Render rTexture1 to rTexture2 (vertical)
        GLfloat texelWidthValue = 0;
        GLfloat texelHeightValue = blurRadiusY / (GLfloat)rTextureSize.height;

        rTexture1->getSprite()->setPosition(CCPointMake(rTextureSize.width / 2, rTextureSize.height / 2));
        rTexture1->getSprite()->setShaderProgram(shader);
        shader->use();
        glUniform1f(texelWidthOffset, texelWidthValue);
        glUniform1f(texelHeightOffset, texelHeightValue);
        ccBlendFunc blend = { GL_ONE, GL_ZERO };
        rTexture1->getSprite()->setBlendFunc(blend);
        rTexture2->beginWithClear(0, 0, 0, 0);
        rTexture1->getSprite()->visit();
        rTexture2->end();
    }
    CHECK_GL_ERROR_DEBUG();

    return rTexture2;
}

CCGLProgram * GAFTextureEffectsConverter::programForGlowShaderWithName(const char * aShaderName, const char * aVertexShaderFile, const char * aFragmentShaderFile)
{
    CCGLProgram *program = CCShaderCache::sharedShaderCache()->programForKey(aShaderName);

    if (!program)
    {
        program = new CCGLProgram();
        bool ok = program->initWithVertexShaderFilename(aVertexShaderFile, aFragmentShaderFile);

        if (ok)
        {
            program->addAttribute("position", kCCVertexAttrib_Position);
            program->addAttribute("inputTextureCoordinate", kCCVertexAttrib_TexCoords);
            program->link();
            program->updateUniforms();
            CHECK_GL_ERROR_DEBUG();
            CCShaderCache::sharedShaderCache()->addProgram(program, aShaderName);
        }
        else
        {
            CCLOGWARN("Cannot load program for %s.", aShaderName);
            return NULL;
        }
    }

    return program;
}

CCGLProgram * GAFTextureEffectsConverter::programForBlurShaderWithName(const char * aShaderName, const char * aVertexShaderFile, const char * aFragmentShaderFile)
{
    CCGLProgram *program = CCShaderCache::sharedShaderCache()->programForKey(aShaderName);
    if (!program)
    {
#if CC_TARGET_PLATFORM == CC_PLATFORM_WP8 || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT && !defined(_DEBUG))
#include "ShadersPrecompiled/GAFPrecompiledShaders.h"

        if (std::string(aShaderName) != "GaussianBlur") //make only for GaussianBlur
        {
            CCAssert(false, "");
            return NULL;
        }

        program = new CCGLProgram();
        program->autorelease();
        program->initWithPrecompiledProgramByteArray((const GLchar*)GaussianBlur, sizeof(GaussianBlur));
        program->addAttribute("position", kCCVertexAttrib_Position);
        program->addAttribute("inputTextureCoordinate", kCCVertexAttrib_TexCoords);
        program->updateUniforms();
        CHECK_GL_ERROR_DEBUG();
        CCShaderCache::sharedShaderCache()->addProgram(program, aShaderName);
#else
        program = new CCGLProgram();
        program->initWithVertexShaderFilename(aVertexShaderFile, aFragmentShaderFile);
        if (program)
        {
            program->addAttribute("position", kCCVertexAttrib_Position);
            program->addAttribute("inputTextureCoordinate", kCCVertexAttrib_TexCoords);
            program->link();
            program->updateUniforms();
            CHECK_GL_ERROR_DEBUG();
            CCShaderCache::sharedShaderCache()->addProgram(program, aShaderName);
        }
        else
        {
            CCLOGWARN("Cannot load program for %s.", aShaderName);
            return NULL;
        }
#endif
    }
    return program;
}

#if 0 //CC_ENABLE_CACHE_TEXTURE_DATA
void _GAFreloadBlurShader()
{
    CCGLProgram *program = CCShaderCache::sharedShaderCache()->programForKey("GaussianBlur");
    if (!program)
    {
        return;
    }
    program->reset();
    program->initWithVertexShaderFilename("Shaders/GaussianBlurVertexShader.vs", "Shaders/GaussianBlurFragmentShader.fs");
    if (program)
    {
        program->addAttribute("position", kCCVertexAttrib_Position);
        program->addAttribute("inputTextureCoordinate", kCCVertexAttrib_TexCoords);
        program->link();
        program->updateUniforms();
        CHECK_GL_ERROR_DEBUG();
    }
    else
    {
        CCAssert(false, "Can not RELOAD GAFTextureEffectsConverter");
    }
    CCLOGERROR("GAFTextureEffectsConverter RELOADED");
}
#endif