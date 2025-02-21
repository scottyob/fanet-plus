# Fanet Plus

=====================================

Fanet is an open protocol created by Juergen Eckert. The current, modern implementation is Fanet+, running over LoRa. You can see what seems to be the golden standard in a reference implementation in his stm32 repo, found with the [specifications here](https://github.com/3s1d/fanet-stm32/blob/master/Src/fanet/radio/protocol.txt). Another good reference [is this pdf](https://github.com/Betschi/FANET/blob/master/Specifications/FANET_Protocol_V1_1_02.pdf)

This library aims at being an extremely simple implementation of a subset of the message types in the Fanet+ protocol spec, designed to run on little endian systems in the Arduino ecosystem. It is not opinionated on what library you use to receive or transmit the packets, but you should be careful to only transmit on the frequencies and duty cycles allowed by your local jurisdiction.

## Message Types and Fanet+ Features

Only a subset of Fanet+ message types and features were implemented (Pull requests welcome!)

| Feature/Message Type          | Supported |
| ----------------------------- | --------- |
| FANet Header                  | ✔️        |
| Extended Header               | ✔️        |
| Extended Header - Dst Address | ✔️        |
| Extended Header - Signature   | ❌        |
| Packet Type Ack               | ✔️        |
| Packet Type Tracking          | ✔️        |
| Packet Type Name              | ✔️        |
| Packet Type Message           | ✔️        |
| Packet Type Service           | ❌        |
| Packet Type Landmarks         | ❌        |
| Packet Type Remote Config     | ❌        |
| Packet Type Ground Tracking   | ✔️        |

## Packet Handler

This library is good for parsing and encoding Fanet messages at the moment, includes a fanet FanetManager, but is currently untested and should be used with care.

## Usage Examples

Use whatever chip and library you wish to rx/tx. These examples are using
[RadioLib](https://github.com/jgromes/RadioLib)

**Receiving a USA Fanet Packet**

```c++

SX1262 radio = new Module((uint32_t)LORA_PIN_LORA_NSS,
                          (uint32_t)LORA_PIN_LORA_DIO_1,
                          (uint32_t)LORA_PIN_LORA_RESET,
                          (uint32_t)LORA_PIN_LORA_BUSY);
volatile bool receivedFlag = false;

void setup() {
    SPI.begin(SPI_CLK, SPI_MISO, SPI_MOSI);
    // set the function that will be called
    // when new packet is received
    radio.setPacketReceivedAction(setFlag);
    radio.begin(920.800f, 500.0f, 7U, 5U, 0xF1, 10U, 8U, 1.6f, false);


// start listening for LoRa packets
    Serial.print(F("[SX1262] Starting to listen ... "));
    state = radio.startReceive();
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("success!"));
    } else {
        Serial.print(F("failed, code "));
        Serial.println(state);
        while (true) {
            delay(10);
        }
    }
}

ICACHE_RAM_ATTR
void setFlag() {
  receivedFlag = true;
}

void loop() {
    if(!receivedFlag) {
        return;
    }

    uint8_t buffer[256];
    auto length = radio.getPacketLength();
    auto state = radio.readData(buffer, 256);
    if (state == RADIOLIB_ERR_NONE) {
        auto rssi = radio.getRSSI();
        auto snr = radio.getSNR();

        // Decode the Fanet+ Packet.
        auto rxPacket = Packet::parse((const char*)buffer, length);
        Serial.println("Got a Fanet+ Packet!");
        Serial.print("Packet Src: 0x");
        Serial.print(rxPacket.header.srcMac.manufacturer, HEX);
        Serial.println(rxPacket.header.srcMac.device, HEX);

        if (rxPacket.header.type == PacketType::Tracking) {
            auto location = etl::get<Tracking>(rxPacket.payload);
            Serial.println((String) "Location: " + location.location.latitude + ", " +
                        location.location.longitude);
            Serial.println((String)"Altitude: " + location.altitude);
        }
    }
}

```

**Sending a location packet**
Much the same as above, and to send, something like this:

```c++
    Serial.println("Sending location packet");
    // Send location once a second
    Packet tx = Packet();
    tx.header.srcMac.manufacturer = 0xFB;
    tx.header.srcMac.device = 1234;
    tx.header.shouldForward = true;
    tx.header.type = PacketType::Tracking;

    // Set the payload to be a tracking packet
    Tracking trackingPayload;
    trackingPayload.aircraftType = AircraftType::Paraglider;
    trackingPayload.altitude = 1000;
    trackingPayload.onlineTracking = false;
    trackingPayload.location.latitude = 37.473358;
    trackingPayload.location.longitude = -122.096409;
    tx.payload = trackingPayload;

    // Sent the buffer out!
    char buffer[256];
    auto len = tx.encode(buffer);
    auto result = radio.transmit(buffer);

```
