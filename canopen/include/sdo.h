/*
This file is part of CanFestival, a library implementing CanOpen Stack. 

Copyright (C): Edouard TISSERANT and Francis DUPIN

See COPYING file for copyrights details.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __sdo_h__
#define __sdo_h__

#include "data.h"

#define SDO_MAX_LENGTH_TRANSFERT       128

typedef void (*SDOtimeoutError_t)(UNS8 line);
void _SDOtimeoutError(UNS8);

typedef void (*SDOCallback_t)(UNS8 nodeId);

/* The 8 bytes data of the SDO */
struct BODY{
    UNS8 data[8];
};

/* The SDO structure ...*/
struct struct_s_SDO {
    UNS8 nodeId;		/*in any case, Node ID of the server (case sender or receiver).*/
    struct BODY body;
};

typedef struct struct_s_SDO s_SDO;

/** Reset of a SDO exchange on timeout. 
  * Send a SDO abort
 */
void SDOTimeoutAlarm(UNS32 id);

/** Reset all sdo buffers
 */
void resetSDO (void);


/** Copy the data received from the SDO line transfert to the object dictionary
 * Returns SDO error code if error. Else, returns 0; 
 */
UNS32 SDOlineToObjdict (UNS8 line);

/** Copy the data from the object dictionary to the SDO line for a network transfert.
 * Returns SDO error code if error. Else, returns 0; 
 */
UNS32 objdictToSDOline (UNS8 line);

/** copy data from an existant line in the argument "* data"
 * Returns 0xFF if error. Else, returns 0; 
 */
UNS8 lineToSDO (UNS8 line, UNS8 nbBytes, UNS8 * data);

/** Add data to an existant line
 * Returns 0xFF if error. Else, returns 0; 
 */
UNS8 SDOtoLine (UNS8 line, UNS8 nbBytes, UNS8 * data);

/** Called when an internal SDO abort occurs.
 * Release the line * Only if server * 
 * If client, the line must be released manually in the core application.
 * The reason of that is to permit the program to read the transfers[][] structure before its reset,
 * because many informations are stored on it : index, subindex, data received or trasmited, ...
 * In all cases, sends a SDO abort.
 * Returns 0
 */
UNS8 failedSDO (UNS8 nodeId, UNS8 whoami, UNS16 index, UNS8 subIndex, UNS32 abortCode);

/** Reset an unused line.
 * 
 */
void resetSDOline (UNS8 line);

/** Initialize some fields of the structure.
 * Returns 0
 */
UNS8 initSDOline (UNS8 line, UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 state);

/** Search for an unused line in the transfers array
 * to store a new SDO.
 * ie a line which value of the field "state" is "SDO_RESET"
 * An unused line have the field "state" at the value SDO_RESET
 * bus_id is hardware dependant
 * whoami : create the line for a SDO_SERVER or SDO_CLIENT.
 * return 0xFF if all the lines are on use. Else, return 0
 */
UNS8 getSDOfreeLine (UNS8 whoami, UNS8 *line);

/** Search for the line, in the transfers array, which contains the
 * beginning of the reception of a fragmented SDO
 * whoami takes 2 values : look for a line opened as SDO_CLIENT or SDO_SERVER
 * bus_id is hardware dependant
 * nodeId correspond to the message node-id 
 * return 0xFF if error.  Else, return 0
 */
UNS8 getSDOlineOnUse (UNS8 nodeId, UNS8 whoami, UNS8 *line);

/** Close a transmission.
 * nodeId : Node id of the server if both server or client
 * whoami : Line opened as SDO_CLIENT or SDO_SERVER
 */
UNS8 closeSDOtransfer (UNS8 nodeId, UNS8 whoami);

/** Bytes in the line structure which must be transmited (or received)
 * bus_id is hardware dependant.
 * return 0.
 */
UNS8 getSDOlineRestBytes (UNS8 line, UNS8 * nbBytes);

/** Store in the line structure the nb of bytes which must be transmited (or received)
 * bus_id is hardware dependant.
 * return 0 if success, 0xFF if error.
 */
UNS8 setSDOlineRestBytes (UNS8 line, UNS8 nbBytes);

/** Transmit a SDO frame on the bus bus_id
 * sdo is a structure which contains the sdo to transmit
 * bus_id is hardware dependant
 * whoami takes 2 values : SDO_CLIENT or SDO_SERVER
 * return canSend(bus_id,&m) or 0xFF if error
 */
UNS8 sendSDO (UNS8 whoami, s_SDO sdo);

/** Transmit a SDO error to the client. The reasons may be :
 * Read/Write to a undefined object
 * Read/Write to a undefined subindex
 * Read/write a not valid length object
 * Write a read only object
 * whoami takes 2 values : SDO_CLIENT or SDO_SERVER
 */
UNS8 sendSDOabort (UNS8 whoami, UNS16 index, UNS8 subIndex, UNS32 abortCode);

/** Treat a SDO frame reception
 * bus_id is hardware dependant
 * call the function sendSDO
 * return 0xFF if error
 *        0x80 if transfert aborted by the server
 *        0x0  ok
 */
UNS8 proceedSDO(Message *m);

/** Used by the application to send a SDO request frame to write the data *data
 * at the index and subIndex indicated
 * in the dictionary of the slave whose node_id is nodeId
 * Count : nb of bytes to write in the dictionnary.
 * datatype (defined in objdictdef.h) : put "visible_string" for strings, 0 for integers or reals or other value.
 * bus_id is hardware dependant
 * return 0xFF if error, else return 0
 */
UNS8 writeNetworkDict (UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 count, UNS8 dataType, void *data, SDOCallback_t Callback); 
/** Used to send a SDO request frame to write in a distant node dictionnary.
 * The function Callback	which must be defined in the user code is called at the
 * end of the exchange. (on succes or abort).
 */       		       
UNS8 writeNetworkDictCallBack (UNS8 nodeId, UNS16 index, 
		       UNS8 subIndex, UNS8 count, UNS8 dataType, void *data, SDOCallback_t Callback);
/** Used by the application to send a SDO request frame to read
 * in the dictionary of a server node whose node_id is ID
 * at the index and subIndex indicated
 * bus_id is hardware dependant
 * datatype (defined in objdictdef.h) : put "visible_string" for strings, 0 for integers or reals or other value.
 * return 0xFF if error, else return 0
 */
UNS8 readNetworkDict (UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 dataType, SDOCallback_t Callback);
		       
/** Used to send a SDO request frame to read in a distant node dictionnary.
 * The function Callback	which must be defined in the user code is called at the
 * end of the exchange. (on succes or abort).
 */   
UNS8 readNetworkDictCallback (UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 dataType, SDOCallback_t Callback);

/** Use this function after a readNetworkDict to get the result.
  Returns : SDO_FINISHED             // data is available
            SDO_ABORTED_RCV          // Transfert failed. (abort SDO received)
            SDO_ABORTED_INTERNAL     // Transfert failed. Internal abort.
            SDO_UPLOAD_IN_PROGRESS   // Data not yet available
	    SDO_DOWNLOAD_IN_PROGRESS // Should not arrive ! 

  dataType (defined in objdictdef.h) : type expected. put "visible_string" for strings, 0 for integers or reals.
  abortCode : 0 = not available. Else : SDO abort code. (received if return SDO_ABORTED_RCV)
  example :
  UNS32 data;
  UNS8 size;
  readNetworkDict(0, 0x05, 0x1016, 1, 0) // get the data index 1016 subindex 1 of node 5
  while (getReadResultNetworkDict (0, 0x05, &data, &size) != SDO_UPLOAD_IN_PROGRESS);
*/
UNS8 getReadResultNetworkDict (UNS8 nodeId, void* data, UNS8 *size, UNS32 * abortCode);

/**
  Use this function after a writeNetworkDict to get the result of the write
  It is mandatory to call this function because it is releasing the line used for the transfer.
  Returns : SDO_FINISHED             // data is available
            SDO_ABORTED_RCV          // Transfert failed. (abort SDO received)
            SDO_ABORTED_INTERNAL     // Transfert failed. Internal abort.
            SDO_DOWNLOAD_IN_PROGRESS // Data not yet available
	    SDO_UPLOAD_IN_PROGRESS   // Should not arrive ! 
  abortCode : 0 = not available. Else : SDO abort code. (received if return SDO_ABORTED_RCV)
  example :
  UNS32 data = 0x50;
  UNS8 size;
  UNS32 abortCode;
  writeNetworkDict(0, 0x05, 0x1016, 1, size, &data) // write the data index 1016 subindex 1 of node 5
  while ( getWriteResultNetworkDict (0, 0x05, &abortCode) != SDO_DOWNLOAD_IN_PROGRESS);  
*/
UNS8 getWriteResultNetworkDict (UNS8 nodeId, UNS32 * abortCode);

#endif
