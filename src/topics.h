#ifndef _TOPICS_H_
#define _TOPICS_H_

/** @name Zmienne wartości topiców
 * 
 */
///@{
/** Znak oznaczający miejsce */
String asp_c = "0";
/** Znak oznaczający urządzenie */
String asm_c = "3";
///@}

/**
 *  Sklejenie początku topicu ze zmiennymi częściami w jeden łańcuch znaków
 */
String location = "asps/asp" + asp_c + "/asm" + asm_c + "/";

/** @name Topici na które przychodzą żądania
 * 
 */
///@{
/** Czy drzwi są otwarte? */
String door_status_request = location + "door/status/request";
/** Otwarcie elektrozamka */
String door_permit = location + "door/permit";
/** Przprowadzenie inwentaryzacji */
String reader_read_request = location + "reader/read/request";
///@}

/** @name Topici na które lodówka przesyła wiadomości
 * 
 */
///@{
/** Informacja czy drzwi są otwarte */
String door_status = location + "door/status";
/** Wynik inwentaryzacji w postaci listy EPC */
String reader_read_tags = location + "reader/read/tags";
///@}

/** @name Topici diagonostyczne
 * 
 */
///@{
/** Timestamp */
String diagnostics_timestamp = location + "diagnostics/timestamp";
///@}

#endif