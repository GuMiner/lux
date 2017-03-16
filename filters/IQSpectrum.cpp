#include <algorithm>
#include "IQSpectrum.h"

IQSpectrum::IQSpectrum(SdrBuffer* dataBuffer) 
    : firstUpdate(true), windowedSincFilter(), FilterBase(dataBuffer)
{
}

// Let's see the IQ graph only within a 3.71 kHz bandwidth. With our current sample rate of 2.4 MHz, we will decimate 648 samples for 1 sample of data.
void IQSpectrum::FormDecimator()
{
    windowedSincFilter.CreateFilter(3710, 648);
    float sum = 0;
    for (unsigned int i = 0; i < windowedSincFilter.kernel.size(); i++)
    {
        // TODO make a filter display pane that takes CustomFilter types for display.
        float offsetX = -10.0f;
        float offsetY = -6.0f;
        float zPos = -30.0f;
        float xEffective = offsetX + ((float)i / (float)windowedSincFilter.kernel.size()) * 8.0f;
        Logger::Log("value: ", windowedSincFilter.kernel[i]);
        filterData.positionBuffer.vertices.push_back(glm::vec3(xEffective, offsetY + windowedSincFilter.kernel[i] * 4.0f, zPos));
        filterData.colorBuffer.vertices.push_back(glm::vec3(1.0f, 1.0f, 0.0f));
    }

    Logger::Log("Formed windowed sinc filter for future decimation.");
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

    glGenVertexArrays(1, &filterData.vao);
    glBindVertexArray(filterData.vao);

    filterData.positionBuffer.Initialize();
    filterData.colorBuffer.Initialize();

    FormDecimator();

    glGenVertexArrays(1, &borderData.vao);
    glBindVertexArray(borderData.vao);

    borderData.positionBuffer.Initialize();
    borderData.colorBuffer.Initialize();
    float displayScale = 6.1f;
    float displayXOffset = 3.0f;
    borderData.positionBuffer.vertices.push_back(glm::vec3(-displayScale + displayXOffset, -displayScale, -30.0f));
    borderData.positionBuffer.vertices.push_back(glm::vec3(-displayScale + displayXOffset, displayScale, -30.0f));
    borderData.colorBuffer.vertices.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
    borderData.colorBuffer.vertices.push_back(glm::vec3(1.0f, 0.0f, 0.0f));

    borderData.positionBuffer.vertices.push_back(glm::vec3(-displayScale + displayXOffset, -displayScale, -30.0f));
    borderData.positionBuffer.vertices.push_back(glm::vec3(displayScale + displayXOffset, -displayScale, -30.0f));
    borderData.colorBuffer.vertices.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
    borderData.colorBuffer.vertices.push_back(glm::vec3(1.0f, 0.0f, 0.0f));

    borderData.positionBuffer.vertices.push_back(glm::vec3(-displayScale + displayXOffset, displayScale, -30.0f));
    borderData.positionBuffer.vertices.push_back(glm::vec3(displayScale + displayXOffset, displayScale, -30.0f));
    borderData.colorBuffer.vertices.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
    borderData.colorBuffer.vertices.push_back(glm::vec3(1.0f, 0.0f, 0.0f));

    borderData.positionBuffer.vertices.push_back(glm::vec3(displayScale + displayXOffset, -displayScale, -30.0f));
    borderData.positionBuffer.vertices.push_back(glm::vec3(displayScale + displayXOffset, displayScale, -30.0f));
    borderData.colorBuffer.vertices.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
    borderData.colorBuffer.vertices.push_back(glm::vec3(1.0f, 0.0f, 0.0f));

    enabled = true;

    return true;
}

void IQSpectrum::Process(std::vector<unsigned char>* block)
{
    Logger::Log("Processing block of ", block->size(), " elements in filter '", GetName(), "'.");

    // Displays the IQ spectrum flattened on the XY plane.
    float displayScale = 6.0f;
    float scale = (1.0f / 127.5f) * displayScale;
    float offsetX = 3.0f;
    float offsetY = 0.0f;
    float zPos = -30.0f;

    graphicsUpdateLock.lock();
    spectrumData.positionBuffer.vertices.clear();
    spectrumData.colorBuffer.vertices.clear();
    Logger::Log("Size block:", block->size(), " decimator:", windowedSincFilter.kernel.size(), ".");
    int samples = block->size() / 2;
    for (unsigned int n = 0; n < samples - windowedSincFilter.kernel.size(); n += windowedSincFilter.kernel.size()) // n < block->size() / 2; n++)
    {
        // TODO determine how to properly decimate IQ signals.
        // TODO this leads to discontinuities at edges. We should grab two buffers and process that to avoid boundary problems.
        float i = 0;
        float q = 0;
        for (unsigned int m = 0; m < windowedSincFilter.kernel.size(); m++)
        {
            // TODO there are weird artefacts using these decimation factors (scaling & descaling).s
            i += ((float)(*block)[(n + m) * 2] - 127.5f) * scale * windowedSincFilter.kernel[m];
            q += ((float)(*block)[(n + m) * 2 + 1] - 127.5f) * scale * windowedSincFilter.kernel[m];
        }
        
        i = std::max(i, -displayScale);
        i = std::min(i, displayScale);
        q = std::max(q, -displayScale);
        q = std::min(q, displayScale);

        // i = ((float)(*block)[n * 2] - 127.5f) * scale;
        // q = ((float)(*block)[n * 2 + 1] - 127.5f) * scale;

        spectrumData.positionBuffer.vertices.push_back(glm::vec3(i + offsetX, q + offsetY, zPos));
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
        if (firstUpdate)
        {
            glBindVertexArray(filterData.vao);

            filterData.positionBuffer.TransferToOpenGl();
            filterData.colorBuffer.TransferToOpenGl();

            glBindVertexArray(borderData.vao);

            borderData.positionBuffer.TransferToOpenGl();
            borderData.colorBuffer.TransferToOpenGl();

            firstUpdate = false;
        }

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

    // Render the IQ spectrum
    glUseProgram(spectrumProgram.programId);
    glBindVertexArray(spectrumData.vao);

    glUniformMatrix4fv(spectrumProgram.projMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
    glUniform1f(spectrumProgram.transparencyLocation, 1.0f);
    glUniform1f(spectrumProgram.pointSizeLocation, 1.0f);

    glDrawArrays(GL_POINTS, 0, spectrumData.lastBufferSize);

    // Render the decimation filter (TODO should be separate)
    glBindVertexArray(filterData.vao);
    glDrawArrays(GL_POINTS, 0, filterData.positionBuffer.vertices.size());

    glBindVertexArray(borderData.vao);
    glDrawArrays(GL_LINES, 0, borderData.positionBuffer.vertices.size());

}

IQSpectrum::~IQSpectrum()
{
    StopFilter();

    glDeleteVertexArrays(1, &spectrumData.vao);
    spectrumData.positionBuffer.Deinitialize();
    spectrumData.colorBuffer.Deinitialize();

    // FYI base destructor is automatically called, which doesn't match with how other languages handle base classes.
}
