#pragma once

enum class MessageType : uint16_t {
    Null = 0x0000,
    ResponseAscii = 0x0001,
    TempHumidityData = 0x0002,
    FetPresenceData = 0x0004,
    PirDataRaw = 0x0005,
    ProxSensor = 0x0007,
    AmbientLightSensor = 0x000A,
    BackplateState = 0x000B,
    RawAdcData = 0x000C,
    //AmbientLightData = 0x000C,
    TfeVersion = 0x0018,
    TfeBuildInfo = 0x0019,
    BackplateModelAndBslId = 0x001d,

    BufferedSensoreData = 0x0022,
    PirMotionEvent = 0x0023,
    ProximityEvent = 0x0025,
    ProximitySensorHighDetail = 0x0027,
    EndOfBuffersMessage = 0x002F,

    FetControl = 0x0082,
    PeriodicStatusRequest = 0x0083,
    FetPresenceAck = 0x008f,
    GetTfeId = 0x0090,
    GetTfeVersion = 0x0098,
    GetTfeBuildInfo = 0x0099,
    GetBackplateModelAndBslId = 0x009d,
    GetBslVersion = 0x009b,
    GetCoProcessorBslVersionInfo = 0x009c,
    GetHardwareVersionAndBackplateName = 0x009e,
    GetSerialNumber = 0x009f,
    SetPowerStealMode = 0x00C0,
    GetHistoricalDataBuffers = 0x00A2,
    AcknowledgeEndOfBuffers = 0x00A3,
    TemperatureLock = 0x00B1,
    Reset = 0x00FF,


};