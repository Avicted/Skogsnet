#include "Includes.h"

global_variable i32 ScreenWidth = 640 * 2;
global_variable i32 ScreenHeight = 360 * 2;
global_variable bool Debug = false;
global_variable long int CPUMemory = 0L;
global_variable usize DataPointCount = 0;
global_variable const char *DataFilePath = "./output.dat";

struct DataPoint
{
    u64 UnixTimestamp;
    f32 TemperatureCelsius = -2000.0f; // lol
    f32 HumidityPercent;
};

global_variable DataPoint *DataPoints = NULL;

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
    for (usize i = 0; i < 100000ULL; ++i)
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
            printf("\t Successfully read latest data point from file at time: %llu\n", DataPoint.UnixTimestamp);
            DataPoints[DataPointCount] = DataPoint;
            DataPointCount++;

            printf("\t DataPointCount: %llu\n", DataPointCount);
        }
    }
}

internal void
GameUpdate(f32 DeltaTime)
{
    HandleWindowResize();
    GetNewData(DeltaTime);
}

internal void
GameRender(f32 DeltaTime)
{
    BeginDrawing();
    {
        ClearBackground(BLACK);

        {
            // Draw a grid in dark grey in x and y axis with spacing of 100 pixels
            for (i32 i = 0; i < ScreenWidth; i += 100)
            {
                DrawLine(i, 0, i, ScreenHeight, DARKGRAY);
            }

            for (i32 i = 0; i < ScreenHeight; i += 100)
            {
                DrawLine(0, i, ScreenWidth, i, DARKGRAY);
            }

            DataPoint *DataPoint = &DataPoints[DataPointCount - 1];
            char Text[256];
            sprintf(Text, "Temperature: %f, Humidity: %f", DataPoint->TemperatureCelsius, DataPoint->HumidityPercent);
            DrawText(Text, 10, 10, 40, WHITE);

            // for (i32 i = 0; i < DataPointCount; ++i);
            for (i32 i = DataPointCount - 1; i >= 0; --i)
            {
                i32 PosX = i * (1.0f * ScreenWidth / DataPointCount);
                i32 PosY = DataPoints[i].TemperatureCelsius * (0.8f * ScreenHeight / 40.0f);
                DrawPixel(PosX * 0.8f + 0.1f * ScreenWidth, 0.9f * ScreenHeight - PosY, RED);

                // Draw lines between the points
                if (i > 0)
                {
                    i32 PrevPosX = (i - 1) * (1.0f * ScreenWidth / DataPointCount);
                    i32 PrevPosY = DataPoints[i - 1].TemperatureCelsius * (0.8f * ScreenHeight / 40.0f);

                    DrawLine(PrevPosX * 0.8f + 0.1f * ScreenWidth, 0.9f * ScreenHeight - PrevPosY, PosX * 0.8f + 0.1f * ScreenWidth, 0.9f * ScreenHeight - PosY, RED);
                }

                PosY = DataPoints[i].HumidityPercent * (0.8f * ScreenHeight / 100.0f);
                DrawPixel(PosX * 0.8f + 0.1f * ScreenWidth, 0.9f * ScreenHeight - PosY, GREEN);

                if (i > 0)
                {
                    i32 PrevPosX = (i - 1) * (1.0f * ScreenWidth / DataPointCount);
                    i32 PrevPosY = DataPoints[i - 1].HumidityPercent * (0.8f * ScreenHeight / 100.0f);

                    DrawLine(PrevPosX * 0.8f + 0.1f * ScreenWidth, 0.9f * ScreenHeight - PrevPosY, PosX * 0.8f + 0.1f * ScreenWidth, 0.9f * ScreenHeight - PosY, GREEN);
                }
            }
        }

        if (Debug)
        {
            DrawFPS(10, 10);
        }
    }

    EndDrawing();
}

internal void
SigIntHandler(i32 Signal)
{
    printf("\tCaught SIGINT, exiting peacefully!\n");
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

    DataPoints = (DataPoint *)calloc(1000000ULL, sizeof(DataPoint));
    CPUMemory += 1000000ULL * sizeof(DataPoint);

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
        CloseWindow(); // Close window and OpenGL context

        printf("Memory used in GigaBytes: %f\n", (f32)CPUMemory / (f32)Gigabytes(1));
        printf("Memory used in MegaBytes: %f\n", (f32)CPUMemory / (f32)Megabytes(1));

        free(DataPoints);

        return (0);
    }