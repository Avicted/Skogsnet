#include "../Includes.h"
#include "raylib_includes.h"

i32 ScreenWidth = 640 * 2;
i32 ScreenHeight = 360 * 2;
bool Debug = false;
global_variable i64 CPUMemory = 0L;
usize DataPointCount = 0;
const char *DataFilePath = "./output.dat";
Camera2D MainCamera = {0};
Font MainFont = {0};

struct DataPoint
{
    u32 UnixTimestamp;
    f32 TemperatureCelsius = 0.0f;
    f32 HumidityPercent;
};

DataPoint *DataPoints = NULL;

internal void
ParseInputArgs(i32 argc, char **argv)
{
    if (argc == 1)
    {
        printf("\tNo input args OK!\n");
        printf("Current working directory: %s\n", GetWorkingDirectory());
        return;
    }

    // If any arg matches the flag: SUMMER_2023_DEBUG
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
ReadDataFromFile(const char *FileName)
{
    // Read the data into DataPoints
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

    // Read the data points
    while (getline(&Line, &LineLength, File) != -1)
    {
        DataPoint *DataPoint = &DataPoints[DataPointCount];
        sscanf(Line, "%llu\t%f\t%f\n", &DataPoint->UnixTimestamp, &DataPoint->TemperatureCelsius, &DataPoint->HumidityPercent);
        DataPointCount++;
    }

    free(Line);

    printf("\t Successfully read %llu data points from file!\n", DataPointCount);

    fclose(File);
}

internal void
PrintData(void)
{
    for (usize i = 0; i < 10000000ULL; ++i)
    {
        DataPoint *DataPoint = &DataPoints[i];
        printf("\tDataPoint[%llu]: UnixTimestamp: %llu, TemperatureCelsius: %f, HumidityPercent: %f\n", i, DataPoint->UnixTimestamp, DataPoint->TemperatureCelsius, DataPoint->HumidityPercent);
    }
}

internal void
GetNewData(f32 DeltaTime)
{
    // Every second we want to read the latest data point from the file DataFilePath and update the chart
    local_persist f32 TimeSinceLastUpdate = 0.0f;

    TimeSinceLastUpdate += DeltaTime;

    if (TimeSinceLastUpdate >= 1.0f) // 16 ms is 60 fps
    {
        TimeSinceLastUpdate = 0.0f;

        // Read the latest data point
        FILE *File = fopen(DataFilePath, "rb");

        if (!File)
        {
            printf("\tFailed to open file: %s\n", DataFilePath);
            return;
        }

        // Read the header
        char *Line = NULL;
        usize LineLength = 0;
        i32 Read = getline(&Line, &LineLength, File);
        Assert(Read != -1);

        // Read the latest data point
        DataPoint DataPoint;
        while (getline(&Line, &LineLength, File) != -1)
        {
            sscanf(Line, "%llu\t%f\t%f\n", &DataPoint.UnixTimestamp, &DataPoint.TemperatureCelsius, &DataPoint.HumidityPercent);
        }

        free(Line);

        fclose(File);

        // If the is any new data point -> Append the latest data point to the end of DataPoints
        if (DataPoint.UnixTimestamp != DataPoints[DataPointCount - 1].UnixTimestamp)
        {
            if (Debug)
            {
                printf("\t Successfully read latest data point from file at time: %llu\n", DataPoint.UnixTimestamp);
            }

            DataPoints[DataPointCount] = DataPoint;
            DataPointCount++;

            printf("\tDataPointCount: %llu\n", DataPointCount);
        }
    }
}

internal void
GameUpdate(f32 DeltaTime)
{
    HandleWindowResize();
    HandleCameraZoom();

    {
        // translate based on right click
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            Vector2 Delta = GetMouseDelta();
            Delta = Vector2Scale(Delta, -1.0f / MainCamera.zoom);

            MainCamera.target = Vector2Add(MainCamera.target, Delta);
        }
    }

    GetNewData(DeltaTime);
}

internal void
GameRender(f32 DeltaTime)
{
    BeginDrawing();
    {
        ClearBackground(BLACK);

        BeginMode2D(MainCamera);

        {
            // Draw the grid lines
            for (i32 i = -40; i <= 40; ++i)
            {
                // Draw the bold lines
                if (i % 5 == 0)
                {
                    DrawLine(0.1f * ScreenWidth, 0.9f * ScreenHeight - i * (0.8f * ScreenHeight / 40.0f), 0.9f * ScreenWidth, 0.9f * ScreenHeight - i * (0.8f * ScreenHeight / 40.0f), LIGHTGRAY);
                }
                else
                {
                    DrawLine(0.1f * ScreenWidth, 0.9f * ScreenHeight - i * (0.8f * ScreenHeight / 40.0f), 0.9f * ScreenWidth, 0.9f * ScreenHeight - i * (0.8f * ScreenHeight / 40.0f), DARKGRAY);
                }
            }

            // Draw the labels
            for (i32 i = -40; i <= 40; ++i)
            {
                if (i % 5 == 0)
                {
                    char Label[16];
                    sprintf(Label, "%d", i);
                    DrawText(Label, ScreenWidth - (ScreenWidth / 16.0f), 0.9f * ScreenHeight - i * (0.8f * ScreenHeight / 40.0f), 16, WHITE);
                }
            }
        }

        {
            // Draw the data points
            for (i32 i = DataPointCount - 1; i >= 0; --i)
            {
                i32 PosX = i * (1.0f * ScreenWidth / DataPointCount);
                i32 PosY = DataPoints[i].TemperatureCelsius * (0.8f * ScreenHeight / 40.0f);

                // Draw lines between the points
                if (i > 0)
                {
                    i32 PrevPosX = (i - 1) * (1.0f * ScreenWidth / DataPointCount);
                    i32 PrevPosY = DataPoints[i - 1].TemperatureCelsius * (0.8f * ScreenHeight / 40.0f);

                    DrawLine(PrevPosX * 0.8f + 0.1f * ScreenWidth, 0.9f * ScreenHeight - PrevPosY, PosX * 0.8f + 0.1f * ScreenWidth, 0.9f * ScreenHeight - PosY, RED);
                }

                PosY = DataPoints[i].HumidityPercent * (0.8f * ScreenHeight / 100.0f);

                if (i > 0)
                {
                    i32 PrevPosX = (i - 1) * (1.0f * ScreenWidth / DataPointCount);
                    i32 PrevPosY = DataPoints[i - 1].HumidityPercent * (0.8f * ScreenHeight / 100.0f);

                    DrawLine(PrevPosX * 0.8f + 0.1f * ScreenWidth, 0.9f * ScreenHeight - PrevPosY, PosX * 0.8f + 0.1f * ScreenWidth, 0.9f * ScreenHeight - PosY, GREEN);
                }
            }
        }
    }

    EndMode2D();

    const char *ProgramVersion = "Skogsnet_v0.0.3";
    DrawTextEx(MainFont, ProgramVersion, {10, 10}, 20, 2, DARKGRAY);

    DrawTextEx(MainFont, ProgramVersion, {12, 12}, 20, 2, ORANGE);

    DataPoint *DataPoint = &DataPoints[DataPointCount - 1];
    char TempText[256];
    sprintf(TempText, "Temperature: %f°C", DataPoint->TemperatureCelsius);
    DrawTextEx(MainFont, TempText, {10, 40}, 20, 2, DARKGRAY);

    DrawTextEx(MainFont, TempText, {12, 42}, 20, 2, RED);

    char HumidityText[256];
    sprintf(HumidityText, "Humidity: %f", DataPoint->HumidityPercent);
    DrawTextEx(MainFont, HumidityText, {10, 70}, 20, 2, DARKGRAY);
    DrawTextEx(MainFont, HumidityText, {12, 72}, 20, 2, GREEN);

    // Info text
    DrawTextEx(MainFont, "Right click to move camera", (Vector2){10, 100}, 12, 2, DARKGRAY);
    DrawTextEx(MainFont, "Right click to move camera", (Vector2){12, 102}, 12, 2, PINK);

    DrawTextEx(MainFont, "Scroll to zoom", (Vector2){10, 120}, 12, 2, DARKGRAY);
    DrawTextEx(MainFont, "Scroll to zoom", (Vector2){12, 122}, 12, 2, PINK);

    if (Debug)
    {
        DrawFPS(10, 10);
    }

    EndDrawing();
}

internal void
CleanupOurStuff(void)
{
    CloseWindow(); // Close window and OpenGL context

    printf("\tMemory used in GigaBytes: %f\n", (f32)CPUMemory / (f32)Gigabytes(1));
    printf("\tMemory used in MegaBytes: %f\n", (f32)CPUMemory / (f32)Megabytes(1));

    free(DataPoints);

    CPUMemory -= 10000000ULL * sizeof(DataPoint);
    printf("Freeing DataPoints: %llu\n", 10000000ULL * sizeof(DataPoint));
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
    MainFont = LoadFontEx("./build/fonts/Super Mario Bros. 2.ttf", 32, 0, 250);

    DataPoints = (DataPoint *)calloc(10000000ULL, sizeof(DataPoint));
    CPUMemory += 10000000ULL * sizeof(DataPoint);

    ReadDataFromFile(DataFilePath);

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