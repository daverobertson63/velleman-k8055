/*******************************************************
 C++ Direct K8055 Replacement for K8055.DLL from Velleman

 Dave Robertson
 VV-Integrate

 Copyright 2025, All Rights Reserved

This software is free to use.

Input packet format

+ -- - +-- - +-- - +-- - +-- - +-- - +-- - +-- - +
| DIn | Sta | A1 | A2 | C1 | C2 |
+-- - +-- - +-- - +-- - +-- - +-- - +-- - +-- - +
DIn = Digital input in high nibble, except for input 3 in 0x01
Sta = Status, Board number + 1
A1 = Analog input 1, 0 - 255
A2 = Analog input 2, 0 - 255
C1 = Counter 1, 16 bits(lsb)
C2 = Counter 2, 16 bits(lsb)

Output packet format

+ -- - +-- - +-- - +-- - +-- - +-- - +-- - +-- - +
| CMD | DIG | An1 | An2 | Rs1 | Rs2 | Dbv | Dbv |
+-- - +-- - +-- - +-- - +-- - +-- - +-- - +-- - +
CMD = Command
DIG = Digital output bitmask
An1 = Analog output 1 value, 0 - 255
An2 = Analog output 2 value, 0 - 255
Rs1 = Reset counter 1, command 3
Rs2 = Reset counter 3, command 4
Dbv = Debounce value for counter 1 and 2, command 1 and 2

Or split by commands

Cmd 0, Reset ? ?
Cmd 1, Set debounce Counter 1
+ -- - +-- - +-- - +-- - +-- - +-- - +-- - +-- - +
| CMD|   |   |   |   |   |Dbv|   |
+-- - +-- - +-- - +-- - +-- - +-- - +-- - +-- - +
Cmd 2, Set debounce Counter 2
+ -- - +-- - +-- - +-- - +-- - +-- - +-- - +-- - +
| CMD|   |   |   |   |   |   |Dbv |
+-- - +-- - +-- - +-- - +-- - +-- - +-- - +-- - +
Cmd 3, Reset counter 1
+ -- - +-- - +-- - +-- - +-- - +-- - +-- - +-- - +
| 3 |   |   |   | 00|   |   |   |
+-- - +-- - +-- - +-- - +-- - +-- - +-- - +-- - +
Cmd 4, Reset counter 2
+ -- - +-- - +-- - +-- - +-- - +-- - +-- - +-- - +
| 4 |   |   |   |   | 00|   |   |
+-- - +-- - +-- - +-- - +-- - +-- - +-- - +-- - +
cmd 5, Set analog / digital
+ -- - +-- - +-- - +-- - +-- - +-- - +-- - +-- - +
| 5 | DIG | An1 | An2|   |   |   |   |
+-- - +-- - +-- - +-- - +-- - +-- - +-- - +-- - +

*/

// Original defs from original lib

#define STR_BUFF 256
#define PACKET_LEN 8

#define K8055_IPID 0x5500
#define VELLEMAN_VENDOR_ID 0x10cf
#define K8055_MAX_DEV 4

#define USB_OUT_EP 0x01	/* USB output endpoint */
#define USB_INP_EP 0x81 /* USB Input endpoint */

#define USB_TIMEOUT 20
#define K8055_ERROR -1

#define DIGITAL_INP_OFFSET 0
#define DIGITAL_OUT_OFFSET 1
#define ANALOG_1_OFFSET 2
#define ANALOG_2_OFFSET 3
#define COUNTER_1_OFFSET 4
#define COUNTER_2_OFFSET 6

#define CMD_RESET 0x00
#define CMD_SET_DEBOUNCE_1 0x01
#define CMD_SET_DEBOUNCE_2 0x01
#define CMD_RESET_COUNTER_1 0x03
#define CMD_RESET_COUNTER_2 0x04
#define CMD_SET_ANALOG_DIGITAL 0x05



#include <fx.h>
#include "k8055.h"

#include "hidapi.h"
#include "mac_support.h"
#include <string.h>
#include <stdlib.h>
#include <limits.h>


#ifdef _WIN32
// Thanks Microsoft, but I know how to use strncpy().
#pragma warning(disable:4996)
#endif

class MainWindow : public FXMainWindow {
	FXDECLARE(MainWindow)

public:
	enum {
		ID_FIRST = FXMainWindow::ID_LAST,
		ID_CONNECT,
		ID_SET_ALL_DIGITAL,
		ID_CLEAR_ALL_DIGITAL,
		ID_DIGITAL_INPUT_1,
		ID_DIGITAL_INPUT_2,
		ID_DIGITAL_INPUT_3,
		ID_DIGITAL_INPUT_4,
		ID_DIGITAL_INPUT_5,
		ID_DIGITAL_OUTPUT_1,
		ID_DIGITAL_OUTPUT_2,
		ID_DIGITAL_OUTPUT_3,
		ID_DIGITAL_OUTPUT_4,
		ID_DIGITAL_OUTPUT_5,
		ID_DIGITAL_OUTPUT_6,
		ID_DIGITAL_OUTPUT_7,
		ID_DIGITAL_OUTPUT_8,
		ID_COUNTER_RESET_1,
		ID_COUNTER_RESET_2,
		ID_DBT1_0,
		ID_DBT1_2,
		ID_DBT1_10,
		ID_DBT1_1000,
		ID_DBT2_0,
		ID_DBT2_2,
		ID_DBT2_10,
		ID_DBT2_1000,
		ID_DISCONNECT,
		ID_RESCAN,
		ID_SEND_OUTPUT_REPORT,
		ID_SEND_FEATURE_REPORT,
		ID_GET_FEATURE_REPORT,
		ID_CLEAR,
		ID_TIMER,
		ID_MAC_TIMER,
		ID_LAST,
		ID_QUIT,
		ID_SK5,
		ID_SK6
	};

private:
	FXList* device_list;

	FXButton* connect_button;
	FXButton* disconnect_button;

	// K8055 buttons

	FXCheckButton* CardAddressSK5;
	FXCheckButton* CardAddressSK6;

	FXButton* set_all_digital;
	FXButton* clear_all_digital;
	FXButton* set_all_analog;
	FXButton* clear_all_analog;
	FXButton* output_test;

	FXButton* reset_counter_1;
	FXButton* reset_counter_2;


	// Check buttons for all digital pin in and out
	FXCheckButton* input_pin_1;
	FXCheckButton* input_pin_2;
	FXCheckButton* input_pin_3;
	FXCheckButton* input_pin_4;
	FXCheckButton* input_pin_5;

	FXCheckButton* output_pin_1;
	FXCheckButton* output_pin_2;
	FXCheckButton* output_pin_3;
	FXCheckButton* output_pin_4;
	FXCheckButton* output_pin_5;
	FXCheckButton* output_pin_6;
	FXCheckButton* output_pin_7;
	FXCheckButton* output_pin_8;

	// Debouce Values
	FXLabel *counter1;
	FXLabel* counter2;

	FXRadioButton* Debounce1Time0ms;
	FXRadioButton* Debounce1Time2ms;
	FXRadioButton* Debounce1Time10ms;
	FXRadioButton* Debounce1Time1000ms;
	
	FXRadioButton* Debounce2Time0ms;
	FXRadioButton* Debounce2Time2ms;
	FXRadioButton* Debounce2Time10ms;
	FXRadioButton* Debounce2Time1000ms;

	FXButton* rescan_button;
	FXButton* output_button;
	FXLabel* connected_label;
	FXTextField* output_text;
	FXTextField* output_len;
	FXButton* feature_button;
	FXButton* get_feature_button;
	FXTextField* feature_text;
	FXTextField* feature_len;
	FXTextField* get_feature_text;
	FXText* input_text;
	FXFont* title_font;

	// Use the data target thing to update the values... 
	FXint AD2 = 0;
	FXint AD1 = 0;
	int AD1Counter = 0;

	FXDataTarget       AD1_target;
	FXDataTarget       AD2_target;

	bool VDeviceConnected = false;

	struct hid_device_info* devices;
	hid_device* connected_device;
	size_t getDataFromTextField(FXTextField* tf, char* buf, size_t len);
	int getLengthFromTextField(FXTextField* tf);


protected:
	MainWindow() {};
public:
	MainWindow(FXApp* a);
	~MainWindow();
	virtual void create();

	
	long onSetAllDigital(FXObject* sender, FXSelector sel, void* ptr);
	long onClearAllDigital(FXObject* sender, FXSelector sel, void* ptr);
	long onConnectNew(FXObject* sender, FXSelector sel, void* ptr);
	long onDigitalInput(FXObject* sender, FXSelector sel, void* ptr);
	long onDigitalOutput(FXObject* sender, FXSelector sel, void* ptr);
	long onCounterReset(FXObject* sender, FXSelector sel, void* ptr);

	
	long onDisconnect(FXObject* sender, FXSelector sel, void* ptr);
	long onRescan(FXObject* sender, FXSelector sel, void* ptr);
	long onSendOutputReport(FXObject* sender, FXSelector sel, void* ptr);
	long onSendFeatureReport(FXObject* sender, FXSelector sel, void* ptr);
	long onGetFeatureReport(FXObject* sender, FXSelector sel, void* ptr);
	long onClear(FXObject* sender, FXSelector sel, void* ptr);
	long onTimeout(FXObject* sender, FXSelector sel, void* ptr);
	long onMacTimeout(FXObject* sender, FXSelector sel, void* ptr);



};

// FOX 1.7 changes the timeouts to all be nanoseconds.
// Fox 1.6 had all timeouts as milliseconds.
#if (FOX_MINOR >= 7)
const int timeout_scalar = 1000 * 1000;
#else
const int timeout_scalar = 1;
#endif

FXMainWindow* g_main_window;


FXDEFMAP(MainWindow) MainWindowMap[] = {
	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_CONNECT, MainWindow::onConnectNew),
	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_SET_ALL_DIGITAL, MainWindow::onSetAllDigital),
	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_CLEAR_ALL_DIGITAL, MainWindow::onClearAllDigital),
	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_DIGITAL_INPUT_1, MainWindow::onDigitalInput),
	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_DIGITAL_INPUT_2, MainWindow::onDigitalInput),
	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_DIGITAL_INPUT_3, MainWindow::onDigitalInput),
	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_DIGITAL_INPUT_4, MainWindow::onDigitalInput),
	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_DIGITAL_INPUT_5, MainWindow::onDigitalInput),

	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_DIGITAL_OUTPUT_1, MainWindow::onDigitalOutput),
	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_DIGITAL_OUTPUT_2, MainWindow::onDigitalOutput),
	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_DIGITAL_OUTPUT_3, MainWindow::onDigitalOutput),
	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_DIGITAL_OUTPUT_4, MainWindow::onDigitalOutput),
	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_DIGITAL_OUTPUT_5, MainWindow::onDigitalOutput),
	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_DIGITAL_OUTPUT_6, MainWindow::onDigitalOutput),
	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_DIGITAL_OUTPUT_7, MainWindow::onDigitalOutput),
	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_DIGITAL_OUTPUT_8, MainWindow::onDigitalOutput),

	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_COUNTER_RESET_1, MainWindow::onCounterReset),
	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_COUNTER_RESET_2, MainWindow::onCounterReset),
	
	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_DISCONNECT, MainWindow::onDisconnect),
	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_RESCAN, MainWindow::onRescan),
	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_SEND_OUTPUT_REPORT, MainWindow::onSendOutputReport),
	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_SEND_FEATURE_REPORT, MainWindow::onSendFeatureReport),
	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_GET_FEATURE_REPORT, MainWindow::onGetFeatureReport),
	FXMAPFUNC(SEL_COMMAND, MainWindow::ID_CLEAR, MainWindow::onClear),
	FXMAPFUNC(SEL_TIMEOUT, MainWindow::ID_TIMER, MainWindow::onTimeout),
	FXMAPFUNC(SEL_TIMEOUT, MainWindow::ID_MAC_TIMER, MainWindow::onMacTimeout),
};

FXIMPLEMENT(MainWindow, FXMainWindow, MainWindowMap, ARRAYNUMBER(MainWindowMap));

MainWindow::MainWindow(FXApp* app)

	: FXMainWindow(app, "K8055 Debug Application", NULL, NULL, DECOR_TITLE | DECOR_MINIMIZE | DECOR_MAXIMIZE | DECOR_CLOSE | DECOR_BORDER | DECOR_STRETCHABLE | DECOR_MENU, 100, 100)
{
	devices = NULL;
	connected_device = NULL;
	VDeviceConnected = false;

	FXHorizontalFrame* contents = new FXHorizontalFrame(this, LAYOUT_SIDE_LEFT | LAYOUT_FILL_Y | LAYOUT_FILL_X | FRAME_GROOVE);

	FXVerticalFrame* vf = new FXVerticalFrame(contents, LAYOUT_SIDE_TOP | LAYOUT_FILL_Y | LAYOUT_FILL_X | FRAME_GROOVE);

	FXVerticalFrame* k1 = new FXVerticalFrame(contents, LAYOUT_FILL_Y | LAYOUT_FILL_X);

	// Just for the Card Address
	FXHorizontalFrame* K8055f = new FXHorizontalFrame(k1, LAYOUT_FILL_X);


	// Digital Copntrols and Card Address
	FXGroupBox* kCardAddress = new FXGroupBox(K8055f, "Card Address", FRAME_THICK | LAYOUT_FILL_X);

	FXHorizontalFrame* k1innerHF = new FXHorizontalFrame(kCardAddress, LAYOUT_FILL_X | LAYOUT_FILL_Y);

	CardAddressSK5 = new FXCheckButton(k1innerHF, "SK5", this, LAYOUT_SIDE_TOP | ID_SK5 | FRAME_RIDGE | ICON_BEFORE_TEXT);
	CardAddressSK6 = new FXCheckButton(k1innerHF, "SK6", this, LAYOUT_SIDE_TOP | ID_SK6 | FRAME_RIDGE | ICON_BEFORE_TEXT);

	// Set default values 
	CardAddressSK5->setCheck(true);
	CardAddressSK6->setCheck(true);



	set_all_digital = new FXButton(k1, "Set All Digital", NULL, this, ID_SET_ALL_DIGITAL, BUTTON_NORMAL | LAYOUT_FILL_X);
	clear_all_digital = new FXButton(k1, "Clear All Digital", NULL, this, ID_CLEAR_ALL_DIGITAL, BUTTON_NORMAL | LAYOUT_FILL_X);

	// Spacer
	new FXSeparator(k1);

	set_all_analog = new FXButton(k1, "Set All Analog", NULL, this, ID_CONNECT, BUTTON_NORMAL | LAYOUT_FILL_X);
	clear_all_analog = new FXButton(k1, "Clear All Analog", NULL, this, ID_CONNECT, BUTTON_NORMAL | LAYOUT_FILL_X);

	new FXSeparator(k1);

	output_test = new FXButton(k1, "Output Test", NULL, this, ID_CONNECT, BUTTON_NORMAL | LAYOUT_FILL_X);

	//input_text = new FXText(new FXHorizontalFrame(innerVF, LAYOUT_FILL_X | LAYOUT_FILL_Y | FRAME_SUNKEN | FRAME_THICK, 0, 0, 0, 0, 0, 0, 0, 0), NULL, 0, LAYOUT_FILL_X | LAYOUT_FILL_Y);
	//input_text->setEditable(false);
	//new FXButton(innerVF, "Clear", NULL, this, ID_CLEAR, BUTTON_NORMAL | LAYOUT_RIGHT);



	// This is thge DA1 slider frame
	FXVerticalFrame* k2 = new FXVerticalFrame(contents, LAYOUT_FILL_Y);

	FXSlider* sliderDA1;
	FXSlider* sliderDA2;
	FXSlider* sliderAD1;
	FXSlider* sliderAD2;


	AD1_target.connect(AD1);
	AD2_target.connect(AD2);

	new FXLabel(k2, "DA1");
	sliderDA1 = new FXSlider(k2, nullptr, 0, FX::SLIDER_INSIDE_BAR | LAYOUT_FILL_Y | LAYOUT_CENTER_X | SLIDER_VERTICAL | LAYOUT_FILL_ROW | LAYOUT_FILL_COLUMN, 0, 0, 100, 400);
	sliderDA1->setRange(0, 255);


	FXVerticalFrame* k3 = new FXVerticalFrame(contents, LAYOUT_FILL_Y | LAYOUT_FILL_X);
	new FXLabel(k3, "DA2");
	sliderDA2 = new FXSlider(k3, nullptr, 0, FX::SLIDER_INSIDE_BAR | LAYOUT_FILL_Y | LAYOUT_CENTER_X | SLIDER_VERTICAL | LAYOUT_FILL_ROW | LAYOUT_FILL_COLUMN, 0, 0, 50, 200);
	sliderDA2->setRange(0, 255);


	//slider = new FXSlider(k2, nullptr, 0, LAYOUT_TOP | LAYOUT_FIX_WIDTH | LAYOUT_FIX_HEIGHT | SLIDER_VERTICAL, 0, 0, 200, 30);

	FXVerticalFrame* k4 = new FXVerticalFrame(contents, LAYOUT_FILL_Y | LAYOUT_FILL_X);
	new FXLabel(k4, "AD1");


	sliderAD1 = new FXSlider(k4, &AD1_target, FXDataTarget::ID_VALUE, FX::SLIDER_INSIDE_BAR | LAYOUT_FILL_Y | LAYOUT_CENTER_X | SLIDER_VERTICAL | LAYOUT_FILL_ROW | LAYOUT_FILL_COLUMN);
	sliderAD1->setRange(0, 255);
	
	sliderAD1->setSlotSize(100);

	FXVerticalFrame* k5 = new FXVerticalFrame(contents, LAYOUT_FILL_Y | LAYOUT_FILL_X);
	new FXLabel(k5, "AD2");
	sliderAD2 = new FXSlider(k5, &AD2_target, FXDataTarget::ID_VALUE, FX::SLIDER_VERTICAL | LAYOUT_FILL_Y | LAYOUT_CENTER_X | SLIDER_VERTICAL | LAYOUT_FILL_ROW | LAYOUT_FILL_COLUMN,1000,1000);
	sliderAD2->setRange(0, 255);



	FXVerticalFrame* k6 = new FXVerticalFrame(contents, LAYOUT_FILL_Y | LAYOUT_FILL_X | FRAME_LINE);

	

	// Digital Input Values
	FXGroupBox* kInputs = new FXGroupBox(k6, "Inputs", FRAME_THICK | LAYOUT_FILL_X);
	FXHorizontalFrame* k6inputHF = new FXHorizontalFrame(kInputs, LAYOUT_FILL_X | LAYOUT_FILL_Y);
	input_pin_1 = new FXCheckButton(k6inputHF, "1", this, ID_DIGITAL_INPUT_1, LAYOUT_SIDE_TOP | FRAME_RIDGE | ICON_BEFORE_TEXT);
	input_pin_2 = new FXCheckButton(k6inputHF, "2", this, ID_DIGITAL_INPUT_2, LAYOUT_SIDE_TOP | FRAME_RIDGE | ICON_BEFORE_TEXT);
	input_pin_3 = new FXCheckButton(k6inputHF, "3", this, ID_DIGITAL_INPUT_2, LAYOUT_SIDE_TOP | FRAME_RIDGE | ICON_BEFORE_TEXT);
	input_pin_4 = new FXCheckButton(k6inputHF, "4", this, ID_DIGITAL_INPUT_2, LAYOUT_SIDE_TOP | FRAME_RIDGE | ICON_BEFORE_TEXT);
	input_pin_5 = new FXCheckButton(k6inputHF, "5", this, ID_DIGITAL_INPUT_2, LAYOUT_SIDE_TOP | FRAME_RIDGE | ICON_BEFORE_TEXT);
	
	FXGroupBox* kOutputs = new FXGroupBox(k6, "Outputs", FRAME_THICK | LAYOUT_FILL_X);
	FXHorizontalFrame* k6outputHF = new FXHorizontalFrame(kOutputs, LAYOUT_FILL_X | LAYOUT_FILL_Y);
	
	output_pin_1 = new FXCheckButton(k6outputHF, "1", this, ID_DIGITAL_OUTPUT_1, LAYOUT_SIDE_TOP | FRAME_RIDGE | ICON_BEFORE_TEXT);
	output_pin_2 = new FXCheckButton(k6outputHF, "2", this, ID_DIGITAL_OUTPUT_2, LAYOUT_SIDE_TOP | FRAME_RIDGE | ICON_BEFORE_TEXT);
	output_pin_3 = new FXCheckButton(k6outputHF, "3", this, ID_DIGITAL_OUTPUT_3, LAYOUT_SIDE_TOP | FRAME_RIDGE | ICON_BEFORE_TEXT);
	output_pin_4 = new FXCheckButton(k6outputHF, "4", this, ID_DIGITAL_OUTPUT_4, LAYOUT_SIDE_TOP | FRAME_RIDGE | ICON_BEFORE_TEXT);
	output_pin_5 = new FXCheckButton(k6outputHF, "5", this, ID_DIGITAL_OUTPUT_5, LAYOUT_SIDE_TOP | FRAME_RIDGE | ICON_BEFORE_TEXT);
	output_pin_6 = new FXCheckButton(k6outputHF, "6", this, ID_DIGITAL_OUTPUT_6, LAYOUT_SIDE_TOP | FRAME_RIDGE | ICON_BEFORE_TEXT);
	output_pin_7 = new FXCheckButton(k6outputHF, "7", this, ID_DIGITAL_OUTPUT_7, LAYOUT_SIDE_TOP | FRAME_RIDGE | ICON_BEFORE_TEXT);
	output_pin_8 = new FXCheckButton(k6outputHF, "8", this, ID_DIGITAL_OUTPUT_8, LAYOUT_SIDE_TOP | FRAME_RIDGE | ICON_BEFORE_TEXT);


	// Contains the counters and debounce info and timers
	FXHorizontalFrame* k7 = new FXHorizontalFrame(k6, LAYOUT_FILL_Y | LAYOUT_FILL_X | FRAME_LINE);
	
	FXGroupBox* Counter1 = new FXGroupBox(k7, "Counter 1", FRAME_THICK | LAYOUT_FILL_X);
	
	counter1 = new FXLabel(Counter1,"0", NULL, JUSTIFY_LEFT | FRAME_GROOVE);
	reset_counter_1 = new FXButton(Counter1, "RESET", NULL, this, ID_COUNTER_RESET_1, BUTTON_NORMAL | LAYOUT_FILL_X);

	FXGroupBox* DBT1 = new FXGroupBox(Counter1, "Debounce Time", FRAME_THICK | LAYOUT_FILL_X);

	Debounce1Time0ms = new FXRadioButton(DBT1, "0ms", this, ID_DBT1_0, LAYOUT_SIDE_TOP | ICON_BEFORE_TEXT);
	Debounce1Time2ms = new FXRadioButton(DBT1, "2ms", this, ID_DBT1_2, LAYOUT_SIDE_TOP | ICON_BEFORE_TEXT);
	Debounce1Time10ms = new FXRadioButton(DBT1, "10ms", this, ID_DBT1_10, LAYOUT_SIDE_TOP | ICON_BEFORE_TEXT);
	Debounce1Time1000ms = new FXRadioButton(DBT1, "1000ms", this, ID_DBT1_1000, LAYOUT_SIDE_TOP | ICON_BEFORE_TEXT);

	

	FXGroupBox* Counter2 = new FXGroupBox(k7, "Counter 2", FRAME_THICK | LAYOUT_FILL_X);
	counter2 = new FXLabel(Counter2, "0", NULL, JUSTIFY_LEFT | FRAME_GROOVE);

	reset_counter_2 = new FXButton(Counter2, "RESET", NULL, this, ID_COUNTER_RESET_2, BUTTON_NORMAL | LAYOUT_FILL_X);

	FXGroupBox* DBT2 = new FXGroupBox(Counter2, "Debounce Time", FRAME_THICK | LAYOUT_FILL_X);
	Debounce2Time0ms = new FXRadioButton(DBT2, "0ms", this, ID_DBT2_0, LAYOUT_SIDE_TOP | ICON_BEFORE_TEXT);
	Debounce2Time2ms = new FXRadioButton(DBT2, "2ms", this, ID_DBT2_2, LAYOUT_SIDE_TOP | ICON_BEFORE_TEXT);
	Debounce2Time10ms = new FXRadioButton(DBT2, "10ms", this, ID_DBT2_10, LAYOUT_SIDE_TOP | ICON_BEFORE_TEXT);
	Debounce2Time1000ms = new FXRadioButton(DBT2, "1000ms", this, ID_DBT2_1000, LAYOUT_SIDE_TOP | ICON_BEFORE_TEXT);




	FXLabel* label = new FXLabel(vf, "K8055 Test Tool");
	title_font = new FXFont(getApp(), "Arial", 14, FXFont::Bold);
	label->setFont(title_font);

	new FXLabel(vf,
		"Select a device and press Connect.", NULL, JUSTIFY_LEFT);
	new FXLabel(vf,
		"Output data bytes can be entered in the Output section, \n"
		"separated by space, comma or brackets. Data starting with 0x\n"
		"is treated as hex. Data beginning with a 0 is treated as \n"
		"octal. All other data is treated as decimal.", NULL, JUSTIFY_LEFT);
	new FXLabel(vf,
		"Data received from the device appears in the Input section.",
		NULL, JUSTIFY_LEFT);
	new FXLabel(vf,
		"Optionally, a report length may be specified. Extra bytes are\n"
		"padded with zeros. If no length is specified, the length is \n"
		"inferred from the data.",
		NULL, JUSTIFY_LEFT);
	new FXLabel(vf, "");

	// Device List and Connect/Disconnect buttons
	FXHorizontalFrame* hf = new FXHorizontalFrame(vf, LAYOUT_FILL_X);
	//device_list = new FXList(new FXHorizontalFrame(hf,FRAME_SUNKEN|FRAME_THICK, 0,0,0,0, 0,0,0,0), NULL, 0, LISTBOX_NORMAL|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0,300,200);
	device_list = new FXList(new FXHorizontalFrame(hf, FRAME_SUNKEN | FRAME_THICK | LAYOUT_FILL_X | LAYOUT_FILL_Y, 0, 0, 0, 0, 0, 0, 0, 0), NULL, 0, LISTBOX_NORMAL | LAYOUT_FILL_X | LAYOUT_FILL_Y, 0, 0, 300, 200);
	FXVerticalFrame* buttonVF = new FXVerticalFrame(hf);
	connect_button = new FXButton(buttonVF, "Connect", NULL, this, ID_CONNECT, BUTTON_NORMAL | LAYOUT_FILL_X);
	disconnect_button = new FXButton(buttonVF, "Disconnect", NULL, this, ID_DISCONNECT, BUTTON_NORMAL | LAYOUT_FILL_X);
	disconnect_button->disable();
	rescan_button = new FXButton(buttonVF, "Re-Scan devices", NULL, this, ID_RESCAN, BUTTON_NORMAL | LAYOUT_FILL_X);
	new FXHorizontalFrame(buttonVF, 0, 0, 0, 0, 0, 0, 0, 50, 0);

	connected_label = new FXLabel(vf, "Disconnected");

	new FXHorizontalFrame(vf);

	// Output Group Box
	FXGroupBox* gb = new FXGroupBox(vf, "Output", FRAME_GROOVE | LAYOUT_FILL_X);
	FXMatrix* matrix = new FXMatrix(gb, 3, MATRIX_BY_COLUMNS | LAYOUT_FILL_X);
	new FXLabel(matrix, "Data");
	new FXLabel(matrix, "Length");
	new FXLabel(matrix, "");

	//hf = new FXHorizontalFrame(gb, LAYOUT_FILL_X);
	output_text = new FXTextField(matrix, 30, NULL, 0, TEXTFIELD_NORMAL | LAYOUT_FILL_X | LAYOUT_FILL_COLUMN);
	output_text->setText("0x05111111 ( Digital all on)");
	output_len = new FXTextField(matrix, 5, NULL, 0, TEXTFIELD_NORMAL | LAYOUT_FILL_X | LAYOUT_FILL_COLUMN);
	output_button = new FXButton(matrix, "Send Output Report", NULL, this, ID_SEND_OUTPUT_REPORT, BUTTON_NORMAL | LAYOUT_FILL_X);
	output_button->disable();
	//new FXHorizontalFrame(matrix, LAYOUT_FILL_X);

	//hf = new FXHorizontalFrame(gb, LAYOUT_FILL_X);
	feature_text = new FXTextField(matrix, 30, NULL, 0, TEXTFIELD_NORMAL | LAYOUT_FILL_X | LAYOUT_FILL_COLUMN);
	feature_len = new FXTextField(matrix, 5, NULL, 0, TEXTFIELD_NORMAL | LAYOUT_FILL_X | LAYOUT_FILL_COLUMN);
	feature_button = new FXButton(matrix, "Send Feature Report", NULL, this, ID_SEND_FEATURE_REPORT, BUTTON_NORMAL | LAYOUT_FILL_X);
	feature_button->disable();

	get_feature_text = new FXTextField(matrix, 30, NULL, 0, TEXTFIELD_NORMAL | LAYOUT_FILL_X | LAYOUT_FILL_COLUMN);
	new FXWindow(matrix);
	get_feature_button = new FXButton(matrix, "Get Feature Report", NULL, this, ID_GET_FEATURE_REPORT, BUTTON_NORMAL | LAYOUT_FILL_X);
	get_feature_button->disable();


	// Input Group Box
	gb = new FXGroupBox(vf, "Input", FRAME_GROOVE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
	FXVerticalFrame* innerVF = new FXVerticalFrame(gb, LAYOUT_FILL_X | LAYOUT_FILL_Y);
	input_text = new FXText(new FXHorizontalFrame(innerVF, LAYOUT_FILL_X | LAYOUT_FILL_Y | FRAME_SUNKEN | FRAME_THICK, 0, 0, 0, 0, 0, 0, 0, 0), NULL, 0, LAYOUT_FILL_X | LAYOUT_FILL_Y);
	input_text->setEditable(false);
	new FXButton(innerVF, "Clear", NULL, this, ID_CLEAR, BUTTON_NORMAL | LAYOUT_RIGHT);





}

MainWindow::~MainWindow()
{
	if (VDeviceConnected)
		CloseDevice();

	VDeviceConnected = false;

	delete title_font;
}

void
MainWindow::create()
{
	FXMainWindow::create();
	show();

	onRescan(NULL, 0, NULL);


#ifdef __APPLE__
	init_apple_message_system();
#endif

	getApp()->addTimeout(this, ID_MAC_TIMER,
		50 * timeout_scalar /*50ms*/);
}

long
MainWindow::onConnectNew(FXObject* sender, FXSelector sel, void* ptr)
{

	if (VDeviceConnected)
		return 1;

	

	FXint cur_item = device_list->getCurrentItem();
	if (cur_item < 0)
		return -1;
	FXListItem* item = device_list->getItem(cur_item);
	if (!item)
		return -1;
	struct hid_device_info* device_info = (struct hid_device_info*)item->getData();
	if (!device_info)
		return -1;

	bool sk5 = CardAddressSK5->getCheck();
	bool sk6 = CardAddressSK6->getCheck();

	int VDeviceNumber = 0;
		
	if (sk5 && sk6) 
		VDeviceNumber = 0;
	if (!sk5 && sk6)
		VDeviceNumber = 1;
	if (sk5 && !sk6)
		VDeviceNumber = 2;
	if (!sk5 && !sk6)
		VDeviceNumber = 3;

	int result = OpenDevice(VDeviceNumber);

	if (result != 0) {
		fprintf(stderr, "Device %d cannot be opened\n", VDeviceNumber);
		input_text->setText("Connected to Velleman Device\n");
		return 0;
	}


	FXString s;
	s.format("Connected to Velleman P8055-1: %04hx:%04hx -", device_info->vendor_id, device_info->product_id);
	s += FXString(" ") + device_info->manufacturer_string;
	s += FXString(" ") + device_info->product_string;
	connected_label->setText(s);
	output_button->enable();
	feature_button->enable();
	get_feature_button->enable();
	connect_button->disable();
	disconnect_button->enable();
	input_text->setText(s + "\n");

	getApp()->addTimeout(this, ID_TIMER,
		5 * timeout_scalar /*5ms*/);

	return 1;

}

long MainWindow::onDigitalInput(FXObject* sender, FXSelector sel, void* ptr)
{

	FXCheckButton* tickbox = (FXCheckButton*)sender;
	bool value = tickbox->getCheck();

	fprintf(stderr, "Value of input dighital selectyed %d\n", value);
		
	return 0;
}


long MainWindow::onDigitalOutput(FXObject* sender, FXSelector sel, void* ptr)
{

	FXCheckButton* tickbox = (FXCheckButton*)sender;
	bool value = tickbox->getCheck();
	int theID = 0;

	switch (FXSELID(sel)) {
		
		case ID_DIGITAL_OUTPUT_1: theID = 1; break;	
		case ID_DIGITAL_OUTPUT_2: theID = 2; break;
		case ID_DIGITAL_OUTPUT_3: theID = 3; break;
		case ID_DIGITAL_OUTPUT_4: theID = 4; break;
		case ID_DIGITAL_OUTPUT_5: theID = 5; break;
		case ID_DIGITAL_OUTPUT_6: theID = 6; break;
		case ID_DIGITAL_OUTPUT_7: theID = 7; break;
		case ID_DIGITAL_OUTPUT_8: theID = 8; break;		
	}

	if (value)
		SetDigitalChannel(theID);
	else
		ClearDigitalChannel(theID);


	return 0;
}




/*
	Disconnect the device - clear logicals
*/

long
MainWindow::onDisconnect(FXObject* sender, FXSelector sel, void* ptr)
{

	// Close using the K8055 Lib
	CloseDevice();
	VDeviceConnected = false;

	connected_device = false;
	connected_label->setText("Disconnected");
	output_button->disable();
	feature_button->disable();
	get_feature_button->disable();
	connect_button->enable();
	disconnect_button->disable();


	input_text->appendText("Device disconnected - timer stopped");
	input_text->setBottomLine(INT_MAX);


	// Stop timer
	getApp()->removeTimeout(this, ID_TIMER);


	return 1;
}

long
MainWindow::onRescan(FXObject* sender, FXSelector sel, void* ptr)
{
	struct hid_device_info* cur_dev;

	device_list->clearItems();

	// List the Devices
	hid_free_enumeration(devices);
	devices = hid_enumerate(0x0, 0x0);
	cur_dev = devices;
	while (cur_dev) {
		// Add it to the List Box.
		FXString s;
		FXString usage_str;
		s.format("%04hx:%04hx -", cur_dev->vendor_id, cur_dev->product_id);
		s += FXString(" ") + cur_dev->manufacturer_string;
		s += FXString(" ") + cur_dev->product_string;
		usage_str.format(" (usage: %04hx:%04hx) ", cur_dev->usage_page, cur_dev->usage);
		s += usage_str;
		FXListItem* li = new FXListItem(s, NULL, cur_dev);
		int vid = 0x10CF;
		int pid = 0x5500;
		for (int i =0; i < 4; i++ ) {
			if (cur_dev->vendor_id == 0x10CF ) {
				device_list->appendItem(li);
				break;
			}
		}

		cur_dev = cur_dev->next;
	}

	if (device_list->getNumItems() == 0)
		device_list->appendItem("*** No Devices Connected ***");
	else {
		device_list->selectItem(0);
	}

	return 1;
}

size_t
MainWindow::getDataFromTextField(FXTextField* tf, char* buf, size_t len)
{
	const char* delim = " ,{}\t\r\n";
	FXString data = tf->getText();
	const FXchar* d = data.text();
	size_t i = 0;

	// Copy the string from the GUI.
	size_t sz = strlen(d);
	char* str = (char*)malloc(sz + 1);
	strcpy(str, d);

	// For each token in the string, parse and store in buf[].
	char* token = strtok(str, delim);
	while (token) {
		char* endptr;
		long int val = strtol(token, &endptr, 0);
		buf[i++] = val;
		token = strtok(NULL, delim);
	}

	free(str);
	return i;
}

/* getLengthFromTextField()
   Returns length:
	 0: empty text field
	>0: valid length
	-1: invalid length */
int
MainWindow::getLengthFromTextField(FXTextField* tf)
{
	long int len;
	FXString str = tf->getText();
	size_t sz = str.length();

	if (sz > 0) {
		char* endptr;
		len = strtol(str.text(), &endptr, 0);
		if (endptr != str.text() && *endptr == '\0') {
			if (len <= 0) {
				FXMessageBox::error(this, MBOX_OK, "Invalid length", "Enter a length greater than zero.");
				return -1;
			}
			return len;
		}
		else
			return -1;
	}

	return 0;
}

long
MainWindow::onSendOutputReport(FXObject* sender, FXSelector sel, void* ptr)
{
	char buf[256];

	unsigned char vPacket[9];	// Velleman Packet

	size_t data_len, len;
	int textfield_len;

	memset(buf, 0x0, sizeof(buf));
	memset(vPacket, 0x0, sizeof(vPacket));

	vPacket[0] = (unsigned char)0x1;
	vPacket[1] = (unsigned char)0x5;
	vPacket[2] = (unsigned char)0xff;
	//vPacket[8] = (unsigned char)0xcc;

	//textfield_len = getLengthFromTextField(output_len);
	//data_len = getDataFromTextField(output_text, buf, sizeof(buf));

	//if (textfield_len < 0) {
	//	FXMessageBox::error(this, MBOX_OK, "Invalid length", "Length field is invalid. Please enter a number in hex, octal, or decimal.");
	//	return 1;
	//}

	//if (textfield_len > sizeof(buf)) {
	//	FXMessageBox::error(this, MBOX_OK, "Invalid length", "Length field is too long.");
	//	return 1;
	//}

	//len = (textfield_len) ? textfield_len : data_len;

	//int res = hid_write(connected_device, (const unsigned char*)buf, len);

	int res = hid_write(connected_device, (const unsigned char*)vPacket, 9);

	if (res < 0) {
		FXMessageBox::error(this, MBOX_OK, "Error Writing", "Could not write to device. Error reported was: %ls", hid_error(connected_device));
	}

	return 1;
}

long
MainWindow::onSendFeatureReport(FXObject* sender, FXSelector sel, void* ptr)
{
	char buf[256];
	size_t data_len, len;
	int textfield_len;

	memset(buf, 0x0, sizeof(buf));
	textfield_len = getLengthFromTextField(feature_len);
	data_len = getDataFromTextField(feature_text, buf, sizeof(buf));

	if (textfield_len < 0) {
		FXMessageBox::error(this, MBOX_OK, "Invalid length", "Length field is invalid. Please enter a number in hex, octal, or decimal.");
		return 1;
	}

	if (textfield_len > sizeof(buf)) {
		FXMessageBox::error(this, MBOX_OK, "Invalid length", "Length field is too long.");
		return 1;
	}

	len = (textfield_len) ? textfield_len : data_len;

	int res = hid_send_feature_report(connected_device, (const unsigned char*)buf, len);
	if (res < 0) {
		FXMessageBox::error(this, MBOX_OK, "Error Writing", "Could not send feature report to device. Error reported was: %ls", hid_error(connected_device));
	}

	return 1;
}

long
MainWindow::onGetFeatureReport(FXObject* sender, FXSelector sel, void* ptr)
{
	char buf[256];
	size_t len;

	memset(buf, 0x0, sizeof(buf));
	len = getDataFromTextField(get_feature_text, buf, sizeof(buf));

	if (len != 1) {
		FXMessageBox::error(this, MBOX_OK, "Too many numbers", "Enter only a single report number in the text field");
	}

	int res = hid_get_feature_report(connected_device, (unsigned char*)buf, sizeof(buf));
	if (res < 0) {
		FXMessageBox::error(this, MBOX_OK, "Error Getting Report", "Could not get feature report from device. Error reported was: %ls", hid_error(connected_device));
	}

	if (res > 0) {
		FXString s;
		s.format("Returned Feature Report. %d bytes:\n", res);
		for (int i = 0; i < res; i++) {
			FXString t;
			t.format("%02hhx ", buf[i]);
			s += t;
			if ((i + 1) % 4 == 0)
				s += " ";
			if ((i + 1) % 16 == 0)
				s += "\n";
		}
		s += "\n";
		input_text->appendText(s);
		input_text->setBottomLine(INT_MAX);
	}

	return 1;
}

long
MainWindow::onClear(FXObject* sender, FXSelector sel, void* ptr)
{
	input_text->setText("");
	return 1;
}

long
MainWindow::onCounterReset(FXObject* sender, FXSelector sel, void* ptr)
{
	input_text->setText("Counter Reset ");
	return 1;
}


/*
	Set all output poins on and LED to match
*/
long MainWindow::onSetAllDigital(FXObject* sender, FXSelector sel, void* ptr)
{

	FXString s;

	s = "Write all Digital pins - LEDs should all light\n";


	input_text->appendText(s);
	input_text->setBottomLine(INT_MAX);

	SetAllDigital();

	return 0;
}


/*
	Clear all Digital pins
*/
long MainWindow::onClearAllDigital(FXObject* sender, FXSelector sel, void* ptr)
{

	FXString s;

	s = "Clear all Digital pins - LEDs will extinguish\n";

	input_text->appendText(s);
	input_text->setBottomLine(INT_MAX);

	ClearAllDigital();

	return 0;
}




long
MainWindow::onTimeout(FXObject* sender, FXSelector sel, void* ptr)
{
	unsigned char buf[256];
	int res = 0;
	unsigned char vPacket[9];
	long d, a1, a2, c1, c2;	// Temp values 

	int result = ReadAllValues(&d, &a1, &a2, &c1, &c2);

	if (result) {
		
		input_text->appendText("not connected or reading values...\n");
		input_text->setBottomLine(INT_MAX);
		
	}

	// Update the digital check boxes
	//int value1 = ReadDigitalChannel(1);

	//d& (1 << (0))) > 0)

	// rval & (1 << (Channel - 1))) > 0)
	//bool value = (bool)(d & (1 << (0))) > 0;
	
	input_pin_1->setCheck((d & (1 << (0))) > 0);
	input_pin_2->setCheck((d & (1 << (1))) > 0);
	input_pin_3->setCheck((d & (1 << (2))) > 0);
	input_pin_4->setCheck((d & (1 << (3))) > 0);
	input_pin_5->setCheck((d & (1 << (4))) > 0);
	
	AD1 = a1;
	AD2 = a2;

	//TODO - Might need to vary this a bit for performance... 
	getApp()->addTimeout(this, ID_TIMER,1);
	return 1;
}

long
MainWindow::onMacTimeout(FXObject* sender, FXSelector sel, void* ptr)
{
#ifdef __APPLE__
	check_apple_events();

	getApp()->addTimeout(this, ID_MAC_TIMER,
		50 * timeout_scalar /*50ms*/);
#endif

	return 1;
}

int main(int argc, char** argv)
{

	FXApp app("Velleman K8055 Development Board", "VV-Integrate");
	app.init(argc, argv);
	g_main_window = new MainWindow(&app);
	app.create();
	app.run();
	return 0;
}
