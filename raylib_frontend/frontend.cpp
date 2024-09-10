#include "../includes.h"
#include "raylib_includes.h"

// --------------------------------------------------------
#define SCREEN_WIDTH 640 * 2
#define SCREEN_HEIGHT 360 * 2

i32 ScreenWidth = 640 * 2;
i32 ScreenHeight = 360 * 2;

bool Debug = false;

global_variable i64 CPUMemory = 0L;

usize DataPointCount = 0;
const char *DataFilePath = "./output.dat";

Camera2D MainCamera;
Font MainFont;
const f64 ScalingFactor = 0.90;

struct DataPoint
{
    u32 UnixTimestamp;
    f32 TemperatureCelsius = 0.0f;
    f32 HumidityPercent;
};

struct CircularBuffer
{
    DataPoint *Buffer; // Pointer to the buffer array
    usize Capacity;    // Maximum number of elements the buffer can hold
    usize Head;        // Index of the most recent element
    usize Tail;        // Index of the oldest element
    usize Count;       // Number of elements currently in the buffer
};

CircularBuffer *DataBuffer;

// --------------------------------------------------------

// Maximum data points 100k
const unsigned long long MAX_DATA_POINTS = 100000ULL;

// Define view window size 10k max
const unsigned long long VIEW_WINDOW_SIZE = 10000ULL;

// Define temperature range and screen range
const float MIN_TEMP = 15.0f;
const float MAX_TEMP = 35.0f;
const float TEMP_SCREEN_MIN = 15.0f;
const float TEMP_SCREEN_MAX = 35.0f;

// Humidity range and screen range
const float MIN_HUMIDITY = 50.0f;
const float MAX_HUMIDITY = 100.0f;
const float HUMIDITY_SCREEN_MIN = 50.0f;
const float HUMIDITY_SCREEN_MAX = 100.0f;

// Define colors for labels
const Color TEMP_LABEL_COLOR = RED;
const Color HUMIDITY_LABEL_COLOR = GREEN;

// Define X position for labels
const f32 LABEL_X_POS = ScalingFactor * ScreenWidth + 5.0f;

// --------------------------------------------------------

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
    Buffer->Head = 0;
    Buffer->Tail = 0;
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

/* internal DataPoint *
GetLatestDataPoint(CircularBuffer *DataBuffer)
{
    if (DataBuffer->Count == 0)
    {
        return NULL; // No data points available
    }

    // Return the latest data point (most recently added)
    return &DataBuffer->Buffer[DataBuffer->Tail];
} */

// Function to update the view window indices based on user input
internal void
UpdateViewWindow(CircularBuffer *buffer, usize *startIndex, usize *endIndex, usize viewSize, f32 zoomFactor, i32 scrollOffset)
{
    usize bufferSize = buffer->Count;
    if (bufferSize == 0)
        return; // No data to display

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
        Vector2 MouseWorldPos = GetScreenToWorld2D(GetMousePosition(), MainCamera);

        // set the offset to where the mouse is
        MainCamera.offset = GetMousePosition();

        // set the target to match, so that the camera maps the world space point under the cursor to the screen space point under the cursor at any zoom
        MainCamera.target = MouseWorldPos;

        // zoom
        MainCamera.zoom += Wheel * 0.125f;
        if (MainCamera.zoom < 0.125f)
        {
            MainCamera.zoom = 0.125f;
        }
    }
}

internal void
ReadDataFromFile(const char *FileName, CircularBuffer *DataBuffer)
{
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
    while (getline(&Line, &LineLength, File) != -1)
    {
        DataPoint DataPoint = {};
        sscanf(Line, "%u\t%f\t%f\n", &DataPoint.UnixTimestamp, &DataPoint.TemperatureCelsius, &DataPoint.HumidityPercent);
        AddDataPoint(DataBuffer, DataPoint);
    }

    free(Line);
    fclose(File);

    printf("\tSuccessfully read data points from file! Total points in buffer: %lu\n", DataBuffer->Count);
}

internal void
PrintData(void)
{
    for (usize i = 0; i < MAX_DATA_POINTS; ++i)
    {
        // DataPoint *DataPoint = &DataPoints[i];
        DataPoint DataPoint = GetDataPoint(DataBuffer, i);
        printf("\tDataPoint[%zu]: UnixTimestamp: %u, TemperatureCelsius: %f, HumidityPercent: %f\n", i, DataPoint.UnixTimestamp, DataPoint.TemperatureCelsius, DataPoint.HumidityPercent);
    }
}

/* internal void
GetNewData(f32 DeltaTime)
{
    local_persist f32 TimeSinceLastUpdate = 0.0f;
    local_persist u64 LastTimestamp = 0;
    local_persist u64 FileOffset = 0;
    local_persist bool FileRead = false;
    local_persist FILE *File = NULL;

    TimeSinceLastUpdate += DeltaTime;

    if (TimeSinceLastUpdate >= 1.0f)
    {
        TimeSinceLastUpdate = 0.0f;

        // Open file only if it's not already open
        if (!FileRead)
        {
            File = fopen(DataFilePath, "rb");
            if (!File)
            {
                printf("\tFailed to open file: %s\n", DataFilePath);
                return;
            }
            FileRead = true;
        }

        // Move to the last read position
        fseek(File, FileOffset, SEEK_SET);

        DataPoint DataPoint;
        bool newData = false;

        // Read data from file
        while (fscanf(File, "%llu\t%f\t%f\n", &DataPoint.UnixTimestamp, &DataPoint.TemperatureCelsius, &DataPoint.HumidityPercent) != EOF)
        {
            if (DataPoint.UnixTimestamp > LastTimestamp)
            {
                LastTimestamp = DataPoint.UnixTimestamp;
                AddDataPoint(DataBuffer, DataPoint);
                newData = true;
            }
        }

        // Update the file offset
        FileOffset = ftell(File);

        // Close file if no new data was found
        if (!newData)
        {
            fclose(File);
            FileRead = false;
        }
    }
} */

internal void
GameUpdate(f32 DeltaTime)
{
    HandleWindowResize();
    HandleCameraZoom();

    // Update view based on user input
    UpdateViewWindow(DataBuffer, &currentView.StartIndex, &currentView.EndIndex, VIEW_WINDOW_SIZE, MainCamera.zoom, 0);

    {
        // translate based on right click
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            Vector2 Delta = GetMouseDelta();
            Delta = Vector2Scale(Delta, -1.0f / MainCamera.zoom);

            MainCamera.target = Vector2Add(MainCamera.target, Delta);
        }
    }

    // @Todo(Victor): To be implemented
    // GetNewData(DeltaTime);
}

internal f32
MapTemperatureToScreen(f32 temperature)
{
    // Clamp temperature to the defined range
    temperature = fmaxf(MIN_TEMP, fminf(MAX_TEMP, temperature));

    // Map temperature to the screen height range
    // Higher temperatures should appear higher on the screen
    return -(temperature - MIN_TEMP) / (MAX_TEMP - MIN_TEMP) * (0.8f * ScreenHeight) + 0.1f * ScreenHeight;
}

internal f32
MapHumidityToScreen(f32 humidity)
{
    // Clamp humidity to the defined range
    humidity = fmaxf(MIN_HUMIDITY, fminf(MAX_HUMIDITY, humidity));

    // Map humidity to screen coordinates with 100% at the top and 0% at the bottom
    return -(humidity - MIN_HUMIDITY) / (MAX_HUMIDITY - MIN_HUMIDITY) * (0.8f * ScreenHeight) + 0.1f * ScreenHeight;
}

internal void
GameRender(f32 DeltaTime)
{
    // Define view window size and zoom factor
    local_persist f32 zoomFactor = 1.0f;
    local_persist i32 scrollOffset = 0;

    // Update view window indices
    usize StartIndex, EndIndex;
    UpdateViewWindow(DataBuffer, &StartIndex, &EndIndex, VIEW_WINDOW_SIZE, zoomFactor, scrollOffset);

    BeginDrawing();
    {
        const Color BackgroundColor = {5, 5, 5, 255};
        ClearBackground(BackgroundColor);

        BeginMode2D(MainCamera);

        // Draw the grid lines for temperature and humidity
        for (i32 i = MIN_TEMP; i <= MAX_TEMP; ++i)
        {
            f32 lineY = roundf(MapTemperatureToScreen(i));
            Color lineColor = (i % 5 == 0) ? LIGHTGRAY : DARKGRAY;
            DrawLine((i32)(0.1f * ScreenWidth), (i32)lineY, (i32)(ScalingFactor * ScreenWidth), (i32)lineY, lineColor);
        }

        // Draw temperature labels on the right side
        for (i32 i = MIN_TEMP; i <= MAX_TEMP; ++i)
        {
            if (i % 5 == 0)
            {
                char Label[16];
                sprintf(Label, "%d°C", i);
                f32 textY = roundf(MapTemperatureToScreen(i) - 10.0f);
                DrawTextEx(MainFont, Label, {(f32)(0.9f * ScreenWidth + 5.0f), textY}, 16.0f, 0.0f, TEMP_LABEL_COLOR);
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
                DrawTextEx(MainFont, Label, {(f32)(ScalingFactor * ScreenWidth + 75.0f), textY}, 16.0f, 0.0f, HUMIDITY_LABEL_COLOR);
            }
        }

        // Draw the data points from the end (newest) backwards
        // Adjust these as needed for the buffer
        usize CurrentIndex = StartIndex;
        usize NextIndex = (StartIndex + 1) % DataBuffer->Capacity;

        // Draw the data points from StartIndex to EndIndex
        while (CurrentIndex != EndIndex)
        {
            DataPoint *Point = &DataBuffer->Buffer[CurrentIndex];
            DataPoint *NextPoint = &DataBuffer->Buffer[NextIndex];

            i32 PosX = (i32)roundf((CurrentIndex - StartIndex) * (1.0f * ScreenWidth / VIEW_WINDOW_SIZE));
            i32 TempPosY = (i32)roundf(MapTemperatureToScreen(Point->TemperatureCelsius));
            i32 HumidPosY = (i32)roundf(MapHumidityToScreen(Point->HumidityPercent));

            i32 NextPosX = (i32)roundf((NextIndex - StartIndex) * (1.0f * ScreenWidth / VIEW_WINDOW_SIZE));
            i32 NextTempPosY = (i32)roundf(MapTemperatureToScreen(NextPoint->TemperatureCelsius));
            i32 NextHumidPosY = (i32)roundf(MapHumidityToScreen(NextPoint->HumidityPercent));

            // Draw temperature lines
            DrawLine((i32)(PosX * 0.8 + 0.1f * ScreenWidth), (i32)TempPosY,
                     (i32)(NextPosX * 0.8 + 0.1f * ScreenWidth), (i32)NextTempPosY, RED);

            // Draw humidity lines
            DrawLine((i32)(PosX * 0.8 + 0.1f * ScreenWidth), (i32)HumidPosY,
                     (i32)(NextPosX * 0.8 + 0.1f * ScreenWidth), (i32)NextHumidPosY, GREEN);

            // Move to the next data point
            CurrentIndex = NextIndex;
            NextIndex = (NextIndex + 1) % DataBuffer->Capacity;
        }

        // Border
        DrawRectangleLinesEx((Rectangle){64, (i32)MapHumidityToScreen(MAX_HUMIDITY) - 100, ScreenWidth, ScreenHeight}, 4.0f, VIOLET);

        EndMode2D();

        // Render UI elements
        const char *ProgramVersion = "Skogsnet_v0.2.0";
        DrawTextEx(MainFont, ProgramVersion, {10, 10}, 20, 2, DARKGRAY);
        DrawTextEx(MainFont, ProgramVersion, {12, 12}, 20, 2, ORANGE);

        if (DataBuffer->Count > 0)
        {
            DataPoint *DataPoint = &DataBuffer->Buffer[(DataBuffer->Tail - 1 + DataBuffer->Capacity) % DataBuffer->Capacity];
            char TempText[256];
            sprintf(TempText, "Temperature: %f°C", DataPoint->TemperatureCelsius);
            DrawTextEx(MainFont, TempText, {10, 40}, 20, 2, DARKGRAY);
            DrawTextEx(MainFont, TempText, {12, 42}, 20, 2, RED);

            char HumidityText[256];
            sprintf(HumidityText, "Humidity: %f", DataPoint->HumidityPercent);
            DrawTextEx(MainFont, HumidityText, {10, 70}, 20, 2, DARKGRAY);
            DrawTextEx(MainFont, HumidityText, {12, 72}, 20, 2, GREEN);
        }

        // Render info text
        DrawTextEx(MainFont, "Right click to move camera", (Vector2){10, 100}, 12, 2, DARKGRAY);
        DrawTextEx(MainFont, "Right click to move camera", (Vector2){12, 102}, 12, 2, PINK);

        DrawTextEx(MainFont, "Scroll to zoom", (Vector2){10, 120}, 12, 2, DARKGRAY);
        DrawTextEx(MainFont, "Scroll to zoom", (Vector2){12, 122}, 12, 2, PINK);

        if (Debug)
        {
            DrawFPS(10, 10);
        }
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

    MainCamera.zoom = 1.0f;
    MainCamera.target = (Vector2){0.0f, -(ScreenHeight - 160)};
    MainFont = LoadFontEx("./raylib_frontend/fonts/Super Mario Bros. 2.ttf", 32, 0, 250);

    // Data Allocation
    DataBuffer = CreateCircularBuffer(MAX_DATA_POINTS);
    CPUMemory += MAX_DATA_POINTS * sizeof(DataPoint);

    ReadDataFromFile(DataFilePath, DataBuffer);

    if (Debug)
    {
        PrintData();
    }

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
