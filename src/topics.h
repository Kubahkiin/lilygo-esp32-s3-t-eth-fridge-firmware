#ifndef _TOPICS_H_
#define _TOPICS_H_

String asp_c = "0";
String asm_c = "3";

String location = "asps/asp" + asp_c + "/asm" + asm_c + "/";

String door_status_request = location + "door/status/request";
String door_permit = location + "door/permit";
String reader_read_request = location + "reader/read/request";
String diagnostics_timestamp = location + "diagnostics/timestamp";

String door_status = location + "door/status";
String reader_read_tags = location + "reader/read/tags";




#endif