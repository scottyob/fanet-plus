#include "unity.h"
#include <iostream>
#include <iomanip>
#include "fanetPacket.h"
#include "etl/array.h"

// Fanet+ packet as sent by SoftRF containing a location packet
etl::array<uint8_t, FANET_MAX_PACKET_SIZE> locationPacket = {
	// Offset 0x00000000 to 0x0000000F
	// Header
    0x41, 
    // Vendor | Src address
    0x07,         0x35, 0x3D,
    
    // Latitude
    0xA3, 0x3E, 0x35, 
    // Longitude
    0xB9, 0x22, 0xA9,
    
    // (Altitude) Tracking, Aircraft Type, Altitude MSB.
    0x10, 0xA0,

    // Speed
	0x00, 
    
    // Climb Rate
    0x02, 
    
    // Heading
    0x25, 
    
    // Extra information...
    0x00
};

void setUp(void) {

}

void tearDown(void) {

}

// Tests that we can parse a location packet
void test_parses(void) {
    auto packet = Fanet::Packet::parse(locationPacket, 16);

    TEST_ASSERT_TRUE(packet.header.type == Fanet::PacketType::Tracking);
    TEST_ASSERT_TRUE(packet.header.shouldForward == true);
    TEST_ASSERT_TRUE(packet.header.hasExtensionHeader == false);
}

void test_encodes(void) {
    // Let's assume that parsing is working
    auto packet = Fanet::Packet::parse(locationPacket, 16);

    etl::array<uint8_t, FANET_MAX_PACKET_SIZE> encodedBuffer = {0};
    packet.encode(encodedBuffer);

    for(int i = 0; i < 16; i++) {
        TEST_ASSERT_EQUAL(locationPacket[i], encodedBuffer[i]);
    }

    // std::cout << "Header: ";
    // for(int i = 0; i < 256; i++) {
    //     std::cout << std::hex << std::setw(2) << std::setfill('0') << "0x" << (int) encodedBuffer[i] << " ";

    //     if(i == 3) {
    //         std::cout << "\nLatLng: \n"; 
    //     }
    //     if(i == 9) {
    //         std::cout << "\nAltitude: \n"; 
    //     }
    //     if(i == 11) {
    //         std::cout << "\nSpeed: \n"; 
    //     }
    //     if(i == 12) {
    //         std::cout << "\nClimb Rate: \n"; 
    //     }
    //     if(i == 13) {
    //         std::cout << "\nHeading: \n"; 
    //     }
    //     if(i == 14) {
    //         std::cout << "\nTurnrate & QneOffset: \n"; 
    //     }

    // }
    // std::cout << std::endl;
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_parses);
    RUN_TEST(test_encodes);
    UNITY_END();
}