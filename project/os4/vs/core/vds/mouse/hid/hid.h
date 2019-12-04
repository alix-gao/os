/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : hid.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __HID_H__
#define __HID_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/

/* HID report item format */
#define HID_ITEM_FORMAT_SHORT 0
#define HID_ITEM_FORMAT_LONG 1

/* Special tag indicating long items */
#define HID_ITEM_TAG_LONG 15

/* HID report descriptor item type (prefix bit 2,3), refer to 6.2.2.2 Short Items */
#define HID_ITEM_TYPE_MAIN 0
#define HID_ITEM_TYPE_GLOBAL 1
#define HID_ITEM_TYPE_LOCAL 2
#define HID_ITEM_TYPE_RESERVED 3

/* HID report descriptor main item tags, refer to 6.2.2.4 Main Items */
#define HID_MAIN_ITEM_TAG_INPUT 8
#define HID_MAIN_ITEM_TAG_OUTPUT 9
#define HID_MAIN_ITEM_TAG_FEATURE 11
#define HID_MAIN_ITEM_TAG_BEGIN_COLLECTION 10
#define HID_MAIN_ITEM_TAG_END_COLLECTION 12

/* HID report descriptor main item contents, refer to 6.2.2.4 Main Items */
#define HID_MAIN_ITEM_CONSTANT 0x001
#define HID_MAIN_ITEM_VARIABLE 0x002
#define HID_MAIN_ITEM_RELATIVE 0x004
#define HID_MAIN_ITEM_WRAP 0x008
#define HID_MAIN_ITEM_NONLINEAR 0x010
#define HID_MAIN_ITEM_NO_PREFERRED 0x020
#define HID_MAIN_ITEM_NULL_STATE 0x040
#define HID_MAIN_ITEM_VOLATILE 0x080
#define HID_MAIN_ITEM_BUFFERED_BYTE 0x100

/* HID report descriptor collection item types, 6.2.2.6 Collection, End Collection Items */
#define HID_COLLECTION_PHYSICAL 0
#define HID_COLLECTION_APPLICATION 1
#define HID_COLLECTION_LOGICAL 2

/* HID report descriptor global item tags, 6.2.2.7 Global Items */
#define HID_GLOBAL_ITEM_TAG_USAGE_PAGE 0
#define HID_GLOBAL_ITEM_TAG_LOGICAL_MINIMUM 1
#define HID_GLOBAL_ITEM_TAG_LOGICAL_MAXIMUM 2
#define HID_GLOBAL_ITEM_TAG_PHYSICAL_MINIMUM 3
#define HID_GLOBAL_ITEM_TAG_PHYSICAL_MAXIMUM 4
#define HID_GLOBAL_ITEM_TAG_UNIT_EXPONENT 5
#define HID_GLOBAL_ITEM_TAG_UNIT 6
#define HID_GLOBAL_ITEM_TAG_REPORT_SIZE 7
#define HID_GLOBAL_ITEM_TAG_REPORT_ID 8
#define HID_GLOBAL_ITEM_TAG_REPORT_COUNT 9
#define HID_GLOBAL_ITEM_TAG_PUSH 10
#define HID_GLOBAL_ITEM_TAG_POP 11

/* HID report descriptor local item tags, 6.2.2.8 Local Items */
#define HID_LOCAL_ITEM_TAG_USAGE 0
#define HID_LOCAL_ITEM_TAG_USAGE_MINIMUM 1
#define HID_LOCAL_ITEM_TAG_USAGE_MAXIMUM 2
#define HID_LOCAL_ITEM_TAG_DESIGNATOR_INDEX 3
#define HID_LOCAL_ITEM_TAG_DESIGNATOR_MINIMUM 4
#define HID_LOCAL_ITEM_TAG_DESIGNATOR_MAXIMUM 5
#define HID_LOCAL_ITEM_TAG_STRING_INDEX 7
#define HID_LOCAL_ITEM_TAG_STRING_MINIMUM 8
#define HID_LOCAL_ITEM_TAG_STRING_MAXIMUM 9
#define HID_LOCAL_ITEM_TAG_DELIMITER 10

/* HID usage tables, refer to Hut1_11 */
#define HID_USAGE_PAGE 0xffff0000

/* 0x????0000 */
#define HID_UP_UNDEFINED    0x0000
#define HID_UP_GENDESK      0x0001
#define HID_UP_SIMULATION   0x0002
#define HID_UP_KEYBOARD     0x0007
#define HID_UP_LED          0x0008
#define HID_UP_BUTTON       0x0009
#define HID_UP_ORDINAL      0x000a
#define HID_UP_CONSUMER     0x000c
#define HID_UP_DIGITIZER    0x000d
#define HID_UP_PID          0x000f
#define HID_UP_CUSTOM       0x00ff
#define HID_UP_MSVENDOR     0xff00
#define HID_UP_HPVENDOR     0xff7f
#define HID_UP_LOGIVENDOR   0xffbc

#define HID_USAGE 0x0000ffff

/* 4 Generic Desktop Page (0x01), 0x0001???? */
#define HID_GD_POINTER      0x0001
#define HID_GD_MOUSE        0x0002
#define HID_GD_JOYSTICK     0x0004
#define HID_GD_GAMEPAD      0x0005
#define HID_GD_KEYBOARD     0x0006
#define HID_GD_KEYPAD       0x0007
#define HID_GD_MULTIAXIS    0x0008
#define HID_GD_X            0x0030
#define HID_GD_Y            0x0031
#define HID_GD_Z            0x0032
#define HID_GD_RX           0x0033
#define HID_GD_RY           0x0034
#define HID_GD_RZ           0x0035
#define HID_GD_SLIDER       0x0036
#define HID_GD_DIAL         0x0037
#define HID_GD_WHEEL        0x0038
#define HID_GD_HATSWITCH    0x0039
#define HID_GD_BUFFER       0x003a
#define HID_GD_BYTECOUNT    0x003b
#define HID_GD_MOTION       0x003c
#define HID_GD_START        0x003d
#define HID_GD_SELECT       0x003e
#define HID_GD_VX           0x0040
#define HID_GD_VY           0x0041
#define HID_GD_VZ           0x0042
#define HID_GD_VBRX         0x0043
#define HID_GD_VBRY         0x0044
#define HID_GD_VBRZ         0x0045
#define HID_GD_VNO          0x0046
#define HID_GD_FEATURE      0x0047
#define HID_GD_UP           0x0090
#define HID_GD_DOWN         0x0091
#define HID_GD_RIGHT        0x0092
#define HID_GD_LEFT         0x0093

/* 16 Digitizers (0x0D), 0x000d???? */
#define HID_DG_DIGITIZER            0x0001
#define HID_DG_PEN                  0x0002
#define HID_DG_LIGHTPEN             0x0003
#define HID_DG_TOUCHSCREEN          0x0004
#define HID_DG_TOUCHPAD             0x0005
#define HID_DG_STYLUS               0x0020
#define HID_DG_PUCK                 0x0021
#define HID_DG_FINGER               0x0022
#define HID_DG_TIPPRESSURE          0x0030
#define HID_DG_BARRELPRESSURE       0x0031
#define HID_DG_INRANGE              0x0032
#define HID_DG_TOUCH                0x0033
#define HID_DG_UNTOUCH              0x0034
#define HID_DG_TAP                  0x0035
#define HID_DG_TABLETFUNCTIONKEY    0x0039
#define HID_DG_PROGRAMCHANGEKEY     0x003a
#define HID_DG_INVERT               0x003c
#define HID_DG_TIPSWITCH            0x0042
#define HID_DG_TIPSWITCH2           0x0043
#define HID_DG_BARRELSWITCH         0x0044
#define HID_DG_ERASER               0x0045
#define HID_DG_TABLETPICK           0x0046

/* as of May 20, 2009 the usages below are not yet in the official USB spec
   but are being pushed by Microsft as described in their paper "Digitizer
   Drivers for Windows Touch and Pen-Based Computers" */
#define HID_DG_CONFIDENCE   0x0047
#define HID_DG_WIDTH        0x0048
#define HID_DG_HEIGHT       0x0049
#define HID_DG_CONTACTID    0x0051
#define HID_DG_INPUTMODE    0x0052
#define HID_DG_DEVICEINDEX  0x0053
#define HID_DG_CONTACTCOUNT 0x0054
#define HID_DG_CONTACTMAX   0x0055

/* HID report types --- Ouch! HID spec says 1 2 3! */
#define HID_INPUT_REPORT 0
#define HID_OUTPUT_REPORT 1
#define HID_FEATURE_REPORT 2

/* HID connect requests */
#define HID_CONNECT_HIDINPUT        0x01
#define HID_CONNECT_HIDINPUT_FORCE  0x02
#define HID_CONNECT_HIDRAW          0x04
#define HID_CONNECT_HIDDEV          0x08
#define HID_CONNECT_HIDDEV_FORCE    0x10
#define HID_CONNECT_FF              0x20
#define HID_CONNECT_DEFAULT         (HID_CONNECT_HIDINPUT|HID_CONNECT_HIDRAW|HID_CONNECT_HIDDEV|HID_CONNECT_FF)

/***************************************************************
 enum define
 ***************************************************************/
enum hid_log_level {
    HID_INFO,
    HID_WARNING,
    HID_ERROR
};
#define hid_dbg(level,fmt,arg...) do { if (HID_INFO <= (level)) { flog(fmt, ##arg); } } while (0)

/***************************************************************
 struct define
 ***************************************************************/
/***************************************************************
 * description : 6.2.2.2 Short Items & 6.2.2.3 Long items
 ***************************************************************/
struct hid_item {
    os_u8 format;
    os_u8 size;
    os_u8 type;
    os_u8 tag;
    union {
        os_u8 u8d;
        os_s8 s8d;
        os_u16 u16d;
        os_s16 s16d;
        os_u32 u32d;
        os_s32 s32d;
        os_u8 *longdata;
    } data;
};

/***************************************************************
 * description :
 ***************************************************************/
struct hid_report_info {
    struct list_node node;

    /* global item */
    os_u32 usage_page;
    os_u32 logical_min;
    os_u32 logical_max;
    os_u32 report_id; /* The Report ID field is 8 bits in length. If no Report ID tags are used in the Report descriptor, there is only one report and the Report ID field is omitted. */
    os_u32 report_size;
    os_u32 report_count;

    /* local item */
#define LOCAL_USAGE_CNT 8
    os_u32 usage[LOCAL_USAGE_CNT];
    os_u8 usage_cnt;
    os_u8 usage_min;
    os_u8 usage_max;
};

/***************************************************************
 * description :
 ***************************************************************/
struct hid_mouse_report_wrap {
    os_u32 x_offset;
    os_u32 y_offset;
    os_u32 z_offset;
    os_u32 wheel_offset;
    os_u8 x_size, y_size, z_size, wheel_size;
    /* bool */
    os_u32 left_offset, left_mask;
    os_u32 right_offset, right_mask;
    os_u32 mid_offset, mid_mask;

    os_u8 data[0]; /* ignored by sizeof() */
};

/***************************************************************
 * description :
 ***************************************************************/
struct hid_device {
    os_u8 *report_descriptor;
    os_u8 pipe;
    os_void *int_buffer; // for interrupt transfer
};

/***************************************************************
 extern function
 ***************************************************************/

#pragma pack()

#endif /* end of header */

