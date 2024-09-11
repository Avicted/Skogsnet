#include "../includes.h"
#include "raylib_includes.h"

// --------------------------------------------------------
#define SCREEN_WIDTH 640 * 2  // @Note(Victor): 2x for 1280
#define SCREEN_HEIGHT 360 * 2 // @Note(Victor): 2x for 720

i32 ScreenWidth = 640 * 2;
i32 ScreenHeight = 360 * 2;

bool Debug = false;

global_variable i64 CPUMemory = 0L;

usize DataPointCount = 0;
const char *DataFilePath = "./output.dat";

Camera2D ScreenSpaceCamera;
Font MainFont;
const f64 ScalingFactor = 0.90;

struct DataPoint
{
    long long unsigned int UnixTimestamp;
    f32 TemperatureCelsius = 0.0f;
    f32 HumidityPercent = 0.0f;
};

struct CircularBuffer
{
    DataPoint *Buffer; // Pointer to the buffer array
    usize Capacity;    // Maximum number of elements the buffer can hold
    usize Head;        // Index of the most recent element
    usize Tail;        // Index of the oldest element
    usize Count;       // Number of elements currently in the buffer
};

// Data
global_variable CircularBuffer *DataBuffer;
global_variable std::unordered_map<f32, f32> temperatureCache;

// Rendering
struct Renderer
{
    RenderTexture2D MainRenderTexture;
    int MainRenderTextureWidth;
    int MainRenderTextureHeight;

    Rectangle MainRenderTextureSourceRec;
    Rectangle MainRenderTextureDestRec;
    Vector2 MainRenderTextureOrigin;

    double virtualRatio;
};

Renderer MainRenderer;

// --------------------------------------------------------

// @Note(Victor): these are dynamically adjusted based on the file line count

// Maximum data points 10k max,
unsigned long long MAX_DATA_POINTS = 10000ULL;

// Define view window size 10k max
unsigned long long VIEW_WINDOW_SIZE = 10000ULL;

bool isLoadingData = false;

// Define temperature range and screen range
const float MIN_TEMP = 15.0f;
const float MAX_TEMP = 35.0f;
const float TEMP_SCREEN_MIN = 15.0f;
const float TEMP_SCREEN_MAX = 35.0f;

// Humidity range and screen range
const float MIN_HUMIDITY = 20.0f;
const float MAX_HUMIDITY = 100.0f;
const float HUMIDITY_SCREEN_MIN = 50.0f;
const float HUMIDITY_SCREEN_MAX = 100.0f;

// Define colors for labels
const Color TEMP_LABEL_COLOR = RED;
const Color HUMIDITY_LABEL_COLOR = GREEN;

// Define X position for labels
const f32 LABEL_X_POS = ScalingFactor * ScreenWidth + 5.0f;

// --------------------------------------------------------
internal f32
MapTemperatureToScreen(f32 temperature)
{
    // Check cache first
    auto it = temperatureCache.find(temperature);
    if (it != temperatureCache.end())
    {
        return it->second;
    }

    // Precompute constants
    const f32 kScreenHeightFactor = 0.8f;
    const f32 kScreenOffset = 0.1f;
    const f32 kRange = MAX_TEMP - MIN_TEMP;
    const f32 kClampedTemperatureOffset = kScreenHeightFactor - kScreenOffset;

    // Clamp temperature to the defined range
    temperature = fmaxf(MIN_TEMP, fminf(MAX_TEMP, temperature));

    // Compute scaled temperature
    f32 scaledTemperature = (temperature - MIN_TEMP) / kRange;

    // Map temperature to the screen height range, inverting the scale
    f32 lineY = (1.0f - scaledTemperature) * kClampedTemperatureOffset * SCREEN_HEIGHT + kScreenOffset * SCREEN_HEIGHT;

    // Store result in cache
    temperatureCache[temperature] = lineY;

    return lineY;
}

internal f32
MapHumidityToScreen(f32 humidity)
{
    // Clamp humidity to the defined range
    humidity = fmaxf(MIN_HUMIDITY, fminf(MAX_HUMIDITY, humidity));

    // Map humidity to screen coordinates with 100% at the top and 0% at the bottom
    f32 lineY = (1.0f - (humidity - MIN_HUMIDITY) / (MAX_HUMIDITY - MIN_HUMIDITY)) * (0.8f * SCREEN_HEIGHT) + 0.1f * SCREEN_HEIGHT;

    return lineY;
}

struct View
{
    usize StartIndex;
    usize EndIndex;
};

View currentView = {0, VIEW_WINDOW_SIZE}; // Initial view window

internal CircularBuffer *
CreateCircularBuffer(usize Capacity)
{
    CircularBuffer *Buffer = (CircularBuffer *)malloc(sizeof(CircularBuffer));

    Buffer->Buffer = (DataPoint *)calloc(Capacity, sizeof(DataPoint));
    Buffer->Capacity = Capacity;
    Buffer->Head = 0; // Head is the most recent element
    Buffer->Tail = 0; // Tail is the oldest element
    Buffer->Count = 0;
    return Buffer;
}

internal void
AddDataPoint(CircularBuffer *Buffer, DataPoint Data)
{
    Buffer->Buffer[Buffer->Head] = Data;
    Buffer->Head = (Buffer->Head + 1) % Buffer->Capacity;

    if (Buffer->Count < Buffer->Capacity)
    {
        Buffer->Count++;
    }
    else
    {
        Buffer->Tail = (Buffer->Tail + 1) % Buffer->Capacity;
    }
}

internal DataPoint
GetDataPoint(CircularBuffer *Buffer, usize Index)
{
    if (Buffer != NULL)
    {
        Assert(Index < Buffer->Count);
    }

    usize ActualIndex = (Buffer->Tail + Index) % Buffer->Capacity;
    return Buffer->Buffer[ActualIndex];
}

internal DataPoint *
GetLatestDataPoint(CircularBuffer *DataBuffer)
{
    return &DataBuffer->Buffer[(DataBuffer->Head - 1 + DataBuffer->Capacity) % DataBuffer->Capacity];
}

// Function to update the view window indices based on user input
internal void
UpdateViewWindow(CircularBuffer *buffer, usize *startIndex, usize *endIndex, usize viewSize, f32 zoomFactor, i32 scrollOffset)
{
    usize bufferSize = buffer->Count;
    if (bufferSize == 0)
    {
        return; // No data to display
    }

    // Adjust view size based on zoom factor
    usize newViewSize = viewSize * zoomFactor;

    // Calculate end index based on tail position
    *endIndex = (buffer->Tail + 1) % buffer->Capacity;

    // Calculate start index with scrolling and boundary wrapping
    *startIndex = (*endIndex + bufferSize - newViewSize + scrollOffset) % buffer->Capacity;

    // Ensure indices are within valid range
    if (*startIndex >= *endIndex)
    {
        *startIndex = (*startIndex + buffer->Capacity) % buffer->Capacity;
    }
}

internal void
ParseInputArgs(i32 argc, char **argv)
{
    if (argc == 1)
    {
        printf("\tNo input args OK!\n");
        printf("\tCurrent working directory: %s\n", GetWorkingDirectory());
        return;
    }

    // If any arg matches the flag: FRONTEND_DEBUG
    for (i32 i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "FRONTEND_DEBUG") == 0)
        {
            printf("\tRunning in DEBUG mode !!!\n");
            Debug = true;
        }
    }
}

internal void
HandleWindowResize(void)
{
    if (IsWindowResized())
    {
        MainRenderer.virtualRatio = (double)GetScreenWidth() / (double)SCREEN_WIDTH;

        MainRenderer.MainRenderTextureOrigin = {0.0f, 0.0f};

        // Update screen space camera
        Vector2 ScreenSpaceCameraOffset = {
            static_cast<float>(GetScreenWidth() / 2.0f),
            static_cast<float>((double)GetScreenHeight() / 2.0f),
        };

        ScreenSpaceCamera.offset = ScreenSpaceCameraOffset;

        Vector2 ScreenSpaceCameraTarget = {
            static_cast<float>(GetScreenWidth() / 2.0f),
            static_cast<float>(GetScreenHeight() / 2.0f),
        };

        ScreenSpaceCamera.target = ScreenSpaceCameraTarget;
        ScreenSpaceCamera.rotation = 0.0f;

        // Update destRec
        Rectangle RenderTextureDest = {
            static_cast<float>(-MainRenderer.virtualRatio),
            static_cast<float>(-MainRenderer.virtualRatio),
            static_cast<float>(GetScreenWidth() + (MainRenderer.virtualRatio)),
            static_cast<float>(GetScreenHeight() + (MainRenderer.virtualRatio)),
        };

        MainRenderer.MainRenderTextureDestRec = RenderTextureDest;
    }

    if (IsWindowResized() && !IsWindowFullscreen())
    {
        ScreenWidth = GetScreenWidth();
        ScreenHeight = GetScreenHeight();
    }

    // check for alt + enter
    if (IsKeyPressed(KEY_ENTER) && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
    {
        // see what display we are on right now
        i32 Display = GetCurrentMonitor();

        if (IsWindowFullscreen())
        {
            // if we are full screen, then go back to the windowed size
            SetWindowSize(ScreenWidth, ScreenHeight);
        }
        else
        {
            // if we are not full screen, set the window size to match the monitor we are on
            SetWindowSize(GetMonitorWidth(Display), GetMonitorHeight(Display));
        }

        // toggle the state
        ToggleFullscreen();

        ScreenWidth = GetScreenWidth();
        ScreenHeight = GetScreenHeight();
    }
}

internal void
HandleCameraZoom(void)
{
    // Zoom based on wheel
    f32 Wheel = GetMouseWheelMove();

    if (Wheel != 0)
    {
        // get the world point that is under the mouse
        Vector2 MouseWorldPos = GetScreenToWorld2D(GetMousePosition(), ScreenSpaceCamera);

        // set the offset to where the mouse is
        ScreenSpaceCamera.offset = GetMousePosition();

        // set the target to match, so that the camera maps the world space point under the cursor to the screen space point under the cursor at any zoom
        ScreenSpaceCamera.target = MouseWorldPos;

        // zoom
        ScreenSpaceCamera.zoom += Wheel * 0.0125f;
        if (ScreenSpaceCamera.zoom < 0.125f)
        {
            ScreenSpaceCamera.zoom = 0.125f;
        }
    }
}

internal void
ReadDataFromFile(const char *FileName, CircularBuffer *DataBuffer)
{
    printf("\tReading data from file: %s\n", FileName);

    long long unsigned int LineCount = 0;

    FILE *File = fopen(FileName, "rb");
    if (!File)
    {
        printf("\tFailed to open file: %s\n", FileName);
        return;
    }

    // Read the header
    char *Line = NULL;
    usize LineLength = 0;
    i32 Read = getline(&Line, &LineLength, File);
    Assert(Read != -1);

    // Read the data points and add them to the circular buffer
    std::vector<std::string> lines;
    std::string line;
    while (getline(&Line, &LineLength, File) != -1)
    {
        // Continue on empty lines
        if (strlen(Line) == 0)
        {
            continue;
        }

        line = Line;
        lines.push_back(line);
    }

    // Reverse the lines to get them in the correct order
    std::reverse(lines.begin(), lines.end());

    // Add the data points to the circular buffer
    for (const std::string &line : lines)
    {
        if (LineCount >= MAX_DATA_POINTS)
        {
            printf("\tReached maximum data points: %llu\n", MAX_DATA_POINTS);
            break;
        }

        DataPoint DataPoint = {};
        sscanf(line.c_str(), "%llu\t%f\t%f\n", &DataPoint.UnixTimestamp, &DataPoint.TemperatureCelsius, &DataPoint.HumidityPercent);

        if (DataPoint.TemperatureCelsius == 0.0f || DataPoint.HumidityPercent == 0.0f)
        {
            printf("\tInvalid data point found!\n");
            continue;
        }

        AddDataPoint(DataBuffer, DataPoint);

        LineCount++;
    }

    free(Line);
    fclose(File);

    printf("\tSuccessfully read %llu data points from file! Total points in buffer: %lu\n", LineCount, DataBuffer->Count);
    MAX_DATA_POINTS = LineCount - 1;
    VIEW_WINDOW_SIZE = LineCount - 1;

    printf("\tData read successfully from file: %s\n", FileName);
}

internal void
PrintData(void)
{
    for (usize i = 0; i < MAX_DATA_POINTS; ++i)
    {
        // DataPoint *DataPoint = &DataPoints[i];
        DataPoint DataPoint = GetDataPoint(DataBuffer, i);
        printf("\tDataPoint[%zu]: UnixTimestamp: %llu, TemperatureCelsius: %f, HumidityPercent: %f\n", i, DataPoint.UnixTimestamp, DataPoint.TemperatureCelsius, DataPoint.HumidityPercent);
    }
}

internal std::vector<std::string>
GetLastNLines(const std::string &filename, size_t numLines)
{
    std::ifstream file(filename, std::ios::binary);
    std::vector<std::string> lines;

    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return lines;
    }

    // Move to the end of the file
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();

    std::streampos pos = fileSize;
    std::string line;
    size_t lineCount = 0;

    // Buffer to read chunks of the file
    const size_t bufferSize = 1024; // Adjust buffer size as needed
    char buffer[bufferSize];

    while (pos > 0 && lineCount < numLines)
    {
        size_t readSize = std::min(bufferSize, static_cast<size_t>(pos));
        file.seekg(static_cast<size_t>(pos) - readSize);
        file.read(buffer, readSize);
        pos -= readSize;

        // Process the buffer in reverse
        for (std::streamoff i = readSize - 1; i >= 0; --i)
        {
            if (buffer[i] == '\n')
            {
                if (!line.empty())
                {
                    std::reverse(line.begin(), line.end());
                    lines.push_back(line);
                    line.clear();
                    ++lineCount;
                    if (lineCount >= numLines)
                    {
                        break;
                    }
                }
            }
            else
            {
                line += buffer[i];
            }
        }
    }

    if (!line.empty())
    {
        std::reverse(line.begin(), line.end());
        lines.push_back(line);
    }

    std::reverse(lines.begin(), lines.end()); // Reverse the lines to get them in correct order

    return lines;
}

internal void
GetNewData(f32 DeltaTime)
{
    isLoadingData = true;

    const i32 numLines = 25;
    std::vector<std::string> recentLines = GetLastNLines(DataFilePath, numLines);

    for (const std::string &line : recentLines)
    {
        DataPoint dataPoint = {};
        sscanf(line.c_str(), "%llu\t%f\t%f\n", &dataPoint.UnixTimestamp, &dataPoint.TemperatureCelsius, &dataPoint.HumidityPercent);

        if (dataPoint.TemperatureCelsius == 0.0f || dataPoint.HumidityPercent == 0.0f)
        {
            printf("\tGetNewData: Invalid data point found!\n");
            continue;
        }

        // Only add the data point if it doesn't already exist in the buffer
        // Search for the data point in the buffer
        bool DataPointExists = false;
        for (usize i = 0; i < DataBuffer->Count; ++i)
        {
            DataPoint ExistingDataPoint = GetDataPoint(DataBuffer, i);
            if (ExistingDataPoint.UnixTimestamp == dataPoint.UnixTimestamp)
            {
                DataPointExists = true;
                break;
            }
        }

        if (DataPointExists)
        {
            // printf("\tGetNewData: Data point already exists in buffer!\n");
            continue;
        }
        else
        {
            printf("\tNew data point: UnixTimestamp: %llu, TemperatureCelsius: %f, HumidityPercent: %f\n", dataPoint.UnixTimestamp, dataPoint.TemperatureCelsius, dataPoint.HumidityPercent);
            AddDataPoint(DataBuffer, dataPoint);
        }
    }

    isLoadingData = false;
}

internal void
GameUpdate(f32 DeltaTime)
{
    HandleWindowResize();
    HandleCameraZoom();

    // Update view based on user input
    UpdateViewWindow(DataBuffer, &currentView.StartIndex, &currentView.EndIndex, VIEW_WINDOW_SIZE, ScreenSpaceCamera.zoom, 0);
    {
        // translate based on right click
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            Vector2 Delta = GetMouseDelta();
            ScreenSpaceCamera.target.x -= Delta.x * 0.5;
            ScreenSpaceCamera.target.y -= Delta.y * 0.5;
        }
    }

    // Get new data every 5 seconds
    local_persist f32 elapsedTime = 0.0f;
    elapsedTime += DeltaTime;
    if (elapsedTime >= 5.0f)
    {
        GetNewData(DeltaTime);
        elapsedTime = 0.0f;
    }

    return;

    // Update VIEW_WINDOW_SIZE based on file size
    FILE *File = fopen(DataFilePath, "r");

    // Count lines in file
    i32 LineCount = 0;
    char *Line = NULL;
    usize LineLength = 0;

    while (getline(&Line, &LineLength, File) != -1)
    {
        LineCount++;
    }

    free(Line);
    fclose(File);

    // Update view window size based on file size
    // printf("\tLineCount: %i\n", LineCount);
    VIEW_WINDOW_SIZE = LineCount - 1;
    MAX_DATA_POINTS = LineCount - 1;
}

internal void
GameRender(f32 DeltaTime)
{
    // Define view window size and zoom factor
    local_persist f32 zoomFactor = 1.0f;
    local_persist i32 scrollOffset = 0;

    // Update view window indices
    usize StartIndex = 0;
    usize EndIndex = 0;
    UpdateViewWindow(DataBuffer, &StartIndex, &EndIndex, VIEW_WINDOW_SIZE, zoomFactor, scrollOffset);

    // Rendering
    BeginDrawing();
    const Color BackgroundColor = {5, 5, 5, 255};

    ClearBackground(BackgroundColor);
    BeginTextureMode(MainRenderer.MainRenderTexture);
    ClearBackground(BLACK);
    BeginMode2D(ScreenSpaceCamera);

    // Draw the grid lines for temperature and humidity
    for (i32 i = MIN_TEMP; i <= MAX_TEMP; ++i)
    {
        f32 lineY = roundf(MapTemperatureToScreen(i));
        Color lineColor = (i % 5 == 0) ? (Color){70, 70, 70, 255} : (Color){40, 40, 40, 255};
        DrawLine((i32)(0.1f * SCREEN_WIDTH), (i32)lineY, (i32)(ScalingFactor * SCREEN_WIDTH), (i32)lineY, lineColor);
    }

    // Draw temperature labels on the right side
    for (i32 i = MIN_TEMP; i <= MAX_TEMP; ++i)
    {
        if (i % 5 == 0)
        {
            char Label[16];
            sprintf(Label, "%d°C", i);
            f32 textY = roundf(MapTemperatureToScreen(i) - 10.0f);
            DrawTextEx(MainFont, Label, {(f32)(0.9f * SCREEN_WIDTH + 5.0f), textY}, 16.0f, 0.0f, TEMP_LABEL_COLOR);
        }
    }

    // Draw humidity labels on the right side
    for (i32 i = MIN_HUMIDITY; i <= MAX_HUMIDITY; i += 10)
    {
        if (i % 10 == 0) // Adjusted to match the increments in the grid
        {
            char Label[16];
            sprintf(Label, "%d%%", i);
            f32 textY = roundf(MapHumidityToScreen(i) - 10.0f);
            DrawTextEx(MainFont, Label, {(f32)(ScalingFactor * SCREEN_WIDTH + 75.0f), textY}, 16.0f, 0.0f, HUMIDITY_LABEL_COLOR);
        }
    }

    // Draw the data points from the end (newest) backwards
    // Adjust these as needed for the buffer
    usize CurrentIndex = StartIndex;
    usize NextIndex = (StartIndex + 1) % DataBuffer->Capacity;

    // Draw the data points from StartIndex to EndIndex
    while (CurrentIndex > EndIndex)
    {
        DataPoint *Point = &DataBuffer->Buffer[CurrentIndex];
        DataPoint *NextPoint = &DataBuffer->Buffer[NextIndex];

        if (Point->UnixTimestamp == 0 || Point->TemperatureCelsius == 0.0f || Point->HumidityPercent == 0.0f)
        {
            printf("\tGameRender->Point: Invalid data point found at index: %lu\n", CurrentIndex);
            break;
        }
        if (NextPoint->UnixTimestamp == 0 || NextPoint->TemperatureCelsius == 0.0f || NextPoint->HumidityPercent == 0.0f)
        {
            // printf("\tGameRender->NextIndex Invalid data point found at index: %lu\n", NextIndex);
            break;
        }

        i32 PosX = (i32)roundf((CurrentIndex - StartIndex) * (1.0f * SCREEN_WIDTH / VIEW_WINDOW_SIZE));
        i32 TempPosY = (i32)roundf(MapTemperatureToScreen(Point->TemperatureCelsius));
        i32 HumidPosY = (i32)roundf(MapHumidityToScreen(Point->HumidityPercent));

        i32 NextPosX = (i32)roundf((NextIndex - StartIndex) * (1.0f * SCREEN_WIDTH / VIEW_WINDOW_SIZE));
        i32 NextTempPosY = (i32)roundf(MapTemperatureToScreen(NextPoint->TemperatureCelsius));
        i32 NextHumidPosY = (i32)roundf(MapHumidityToScreen(NextPoint->HumidityPercent));

        // Draw temperature lines
        DrawLine((i32)(PosX * 0.8 + 0.1f * SCREEN_WIDTH), (i32)TempPosY,
                 (i32)(NextPosX * 0.8 + 0.1f * SCREEN_WIDTH), (i32)NextTempPosY, RED);

        // Draw humidity lines
        DrawLine((i32)(PosX * 0.8 + 0.1f * SCREEN_WIDTH), (i32)HumidPosY,
                 (i32)(NextPosX * 0.8 + 0.1f * SCREEN_WIDTH), (i32)NextHumidPosY, GREEN);

        // Move to the next data point
        CurrentIndex = NextIndex;
        NextIndex = (NextIndex + 1) % DataBuffer->Capacity;
    }

    // Border
    Rectangle Border = (Rectangle){64, (i32)MapHumidityToScreen(MAX_HUMIDITY) - 100, SCREEN_WIDTH, SCREEN_HEIGHT};
    DrawRectangleLinesEx(Border, 4.0f, VIOLET);

    EndTextureMode();
    EndMode2D();

    // Draw the render texture to the actual screen
    BeginDrawing();
    ClearBackground(BLACK); // Clear the screen before drawing the texture

    BeginMode2D(ScreenSpaceCamera);

    DrawTexturePro(
        MainRenderer.MainRenderTexture.texture,
        MainRenderer.MainRenderTextureSourceRec, // Source rectangle in the texture
        MainRenderer.MainRenderTextureDestRec,   // Destination rectangle on the screen
        MainRenderer.MainRenderTextureOrigin,    // Origin for the rotation
        0.0f,                                    // Rotation angle
        WHITE                                    // Tint color
    );

    EndMode2D();

    // Render UI elements
    {
        const char *ProgramVersion = "Skogsnet_v0.3.0";
        DrawTextEx(MainFont, ProgramVersion, {10, 10}, 20, 2, DARKGRAY);
        DrawTextEx(MainFont, ProgramVersion, {12, 12}, 20, 2, ORANGE);

        if (DataBuffer->Count > 0)
        {
            DataPoint *DataPoint = GetLatestDataPoint(DataBuffer); // &DataBuffer->Buffer[(DataBuffer->Tail - 1 + DataBuffer->Capacity) % DataBuffer->Capacity];
            char TempText[256];
            sprintf(TempText, "Temperature: %f°C", DataPoint->TemperatureCelsius);
            DrawTextEx(MainFont, TempText, {10, 40}, 20, 2, DARKGRAY);
            DrawTextEx(MainFont, TempText, {12, 42}, 20, 2, RED);

            char HumidityText[256];
            sprintf(HumidityText, "Humidity: %f", DataPoint->HumidityPercent);
            DrawTextEx(MainFont, HumidityText, {10, 70}, 20, 2, DARKGRAY);
            DrawTextEx(MainFont, HumidityText, {12, 72}, 20, 2, GREEN);
        }

        // DEBUG --
        // ClearBackground(MAGENTA);
        // Draw a rectangle for the render texture size border
        // DrawRectangleLinesEx((Rectangle){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT}, 4, RED);

        // Render info text top right corner
        DrawTextEx(MainFont, "Right click to move camera", {10, 100}, 12, 2, DARKGRAY);
        DrawTextEx(MainFont, "Right click to move camera", (Vector2){12, 102}, 12, 2, PINK);

        DrawTextEx(MainFont, "Scroll to zoom", (Vector2){10, 120}, 12, 2, DARKGRAY);
        DrawTextEx(MainFont, "Scroll to zoom", (Vector2){12, 122}, 12, 2, PINK);

        int fps = GetFPS();
        char fpsText[256];
        sprintf(fpsText, "FPS: %d", fps);
        DrawTextEx(MainFont, fpsText, (Vector2){10, 140}, 12, 2, LIGHTGRAY);
    }

    EndDrawing();
}

internal void
CleanupOurStuff(void)
{
    CloseWindow(); // Close window and OpenGL context

    printf("\tMemory used in GigaBytes: %f\n", (f32)CPUMemory / (f32)Gigabytes(1));
    printf("\tMemory used in MegaBytes: %f\n", (f32)CPUMemory / (f32)Megabytes(1));

    free(DataBuffer->Buffer);

    CPUMemory -= MAX_DATA_POINTS * sizeof(DataPoint);
    printf("Freeing DataPoints: %llu\n", MAX_DATA_POINTS * sizeof(DataPoint));
}

internal void
RenderLoadingScreen(void)
{
    BeginDrawing();
    ClearBackground(BLACK);

    // Center screen Loading data...
    DrawText("Loading data...", ScreenWidth / 2 - MeasureText("Loading data...", 20) / 2, ScreenHeight / 2 - 10, 20, WHITE);

    EndDrawing();
}

internal void
SigIntHandler(i32 Signal)
{
    printf("\tCaught SIGINT, exiting peacefully!\n");

    CleanupOurStuff();

    exit(0);
}

int main(int argc, char **argv)
{
    signal(SIGINT, SigIntHandler);

    ParseInputArgs(argc, argv);

    printf("\tHello from raylib_frontend!\n");

    // Raylib
    {
        SetTraceLogLevel(LOG_WARNING);
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
        InitWindow(ScreenWidth, ScreenHeight, "Frontend");

#if defined(PLATFORM_WEB)
        emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
        SetTargetFPS(60);
    }

    MainRenderer.MainRenderTexture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    MainRenderer.virtualRatio = (double)GetScreenWidth() / (double)SCREEN_WIDTH;
    MainRenderer.MainRenderTextureOrigin = {0.0f, 0.0f};

    // The target's height is flipped (in the source Rectangle), due to OpenGL reasons
    MainRenderer.MainRenderTextureSourceRec = {
        0.0f,
        0.0f,
        static_cast<float>(MainRenderer.MainRenderTexture.texture.width),
        static_cast<float>(-MainRenderer.MainRenderTexture.texture.height),
    };

    MainRenderer.MainRenderTextureDestRec = {
        static_cast<float>(-MainRenderer.virtualRatio),
        static_cast<float>(MainRenderer.virtualRatio),
        static_cast<float>(GetScreenWidth() + (MainRenderer.virtualRatio)),
        static_cast<float>(GetScreenHeight() + (MainRenderer.virtualRatio)),
    };

    ScreenSpaceCamera.rotation = 0.0f;
    ScreenSpaceCamera.zoom = 1.0f; // @Note(Avic): Zoom factor
    ScreenSpaceCamera.rotation = 0.0f;
    ScreenSpaceCamera.target = {0.0f, 0.0f}; // (Vector2){0.0f, -(ScreenHeight - 160)};
    ScreenSpaceCamera.offset = {0.0f, 0.0f};

    MainFont = LoadFontEx("./raylib_frontend/fonts/Super Mario Bros. 2.ttf", 32, 0, 250);

    // Data Allocation
    DataBuffer = CreateCircularBuffer(MAX_DATA_POINTS);
    CPUMemory += MAX_DATA_POINTS * sizeof(DataPoint);

    ReadDataFromFile(DataFilePath, DataBuffer);

    if (Debug)
    {
        PrintData();
    }

    RenderLoadingScreen();

    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        f32 DeltaTime = GetFrameTime();
        GameUpdate(DeltaTime);
        GameRender(DeltaTime);
    }
#endif
        CleanupOurStuff();

        return (0);
    }
