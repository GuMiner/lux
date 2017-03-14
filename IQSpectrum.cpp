#include "IQSpectrum.h"



IQSpectrum::IQSpectrum(SdrBuffer* dataBuffer) 
    : FilterBase(dataBuffer)
{
}

bool IQSpectrum::LoadGraphics(ShaderFactory* shaderFactory)
{
    Logger::Log("Loading the point rendering shading program...");
    if (!shaderFactory->CreateShaderProgram("pointRender", &spectrumProgram.programId))
    {
        Logger::LogError("Failed to load the point rendering shader; cannot continue.");
        return false;
    }

    spectrumProgram.projMatrixLocation = glGetUniformLocation(spectrumProgram.programId, "projMatrix");
    spectrumProgram.pointSizeLocation = glGetUniformLocation(spectrumProgram.programId, "pointSize");
    spectrumProgram.transparencyLocation = glGetUniformLocation(spectrumProgram.programId, "transparency");

    glGenVertexArrays(1, &spectrumData.vao);
    glBindVertexArray(spectrumData.vao);

    spectrumData.positionBuffer.Initialize();
    spectrumData.colorBuffer.Initialize();
    spectrumData.lastBufferSize = 0;
    enabled = true;

    return true;
}

void IQSpectrum::Process(std::vector<unsigned char>* block)
{
    Logger::Log("Processing block of ", block->size(), " elements in filter '", GetName(), "'.");

    // Displays the IQ spectrum flattened on the XY plane.
    float scale = (1.0f / 256.0f) * 12.0f;
    float offsetX = -6.0f;
    float offsetY = -6.0f;
    float zPos = -30.0f;

    graphicsUpdateLock.lock();
    spectrumData.positionBuffer.vertices.clear();
    spectrumData.colorBuffer.vertices.clear();
    for (unsigned int n = 0; n < 200 && n < block->size() / 2; n++)
    {
        float i = (*block)[n * 2];
        float q = (*block)[n * 2 + 1];
        spectrumData.positionBuffer.vertices.push_back(glm::vec3(i * scale + offsetX, q * scale + offsetY, zPos));
        spectrumData.colorBuffer.vertices.push_back(glm::vec3(0.50f + i * (1.0f / 512.0f), 0.50f + q * (1.0f / 512.0f), 1.0f));
    }

    // Current view plane is at NEGATIVE 30, extending roughtly 17x10 (center to edges). Thats ... the reverse of what it should be. To investigate later.
    // for (int i = 0; i < 20; i += 1)
    // {
    //     for (int j = 0; j < 20; j += 1)
    //     {
    //         for (int k = -30; k < 0; k += 10)
    //         {              
    //             float xF = (float)i / 200.0f + 0.50f;
    //             float yF = (float)j / 200.0f + 0.50f;
    //             float zF = (float)k / 200.0f + 0.50f;
    //             spectrumData.positionBuffer.vertices.push_back(glm::vec3(i, j, k));
    //             spectrumData.colorBuffer.vertices.push_back(glm::vec3(xF, yF, zF));
    //         }
    //     }
    // }
    graphicsUpdateLock.unlock();

    updateGraphics = true;
}

void IQSpectrum::Render(glm::mat4& projectionMatrix)
{
    if (updateGraphics)
    {
        // Graphics updates must be on the graphics thread.
        glBindVertexArray(spectrumData.vao);

        graphicsUpdateLock.lock();
        spectrumData.positionBuffer.TransferToOpenGl();
        spectrumData.colorBuffer.TransferToOpenGl();
        spectrumData.lastBufferSize = spectrumData.positionBuffer.vertices.size();
        graphicsUpdateLock.unlock();

        updateGraphics = false;
    }
    else if (spectrumData.lastBufferSize == 0)
    {
        return;
    }

    glUseProgram(spectrumProgram.programId);
    glBindVertexArray(spectrumData.vao);

    glUniformMatrix4fv(spectrumProgram.projMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
    glUniform1f(spectrumProgram.transparencyLocation, 1.0f);
    glUniform1f(spectrumProgram.pointSizeLocation, 5.0f);

    
    glDrawArrays(GL_POINTS, 0, spectrumData.lastBufferSize);
}

IQSpectrum::~IQSpectrum()
{
    StopFilter();

    glDeleteVertexArrays(1, &spectrumData.vao);
    spectrumData.positionBuffer.Deinitialize();
    spectrumData.colorBuffer.Deinitialize();

    // FYI base destructor is automatically called, which doesn't match with how other languages handle base classes.
}
