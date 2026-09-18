/********************************************************************/
/** MidiTimeCode synchronisation. 
	Sende und Empfangsroutinen zu MTC.
	
	Bei Programmstart wird init_mtc aufgerufen und 
	der Frame-Typ initialisiert. 

	Da bei angeschaltetem Multi-Play mehrere Objekte gestartet 
	werden k”nnen, aber es immer nur ein Objekt gibt das von 
	aužen gesteuert werden kann, wird in der Variable 
	"seqi->remote_so" das aktuelle Steuer-Objekt vermerkt. 
	An dieses Objekt ist auch die Synchronisation gekoppelt. 
	
**/	
/********************************************************************/

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"

#include "realtim4.h"
#include "realtspc.h"

#include "msh_unit.h"

/** Statics **/

typedef struct _sequi {
} SEQUI;

LOCAL SEQUI sequi;

/** Send MTC **/
STATIC INT   stimh, stimm, stims, stimf;
INT	 		 mtc_count;
LONG  		 mtc_out_time;

#ifndef RCV_MTC_BY_MIDI_SHARE

/** Receive MTC **/
STATIC INT   mtc_val[ 9 ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
STATIC INT	 mtc_lock = 0;
STATIC LONG  mtc_timer;
LONG  mtc_fr[   10 ] = { 24L, 0L, 25L, 0L, 30L, 0L, 30L, 0L, 25L };
STATIC LONG  mtc_dela[ 10 ] = { 83L, 0L, 80L, 0L, 67L, 0L, 67L, 0L, 80L };

#endif RCV_MTC_BY_MIDI_SHARE

/** Sync Receive MTC **/
LONG  		 old_mtc_time;
INT 		 frameval[] = { 24, 25, 30, 30, 100 };


/********************************************************************/
/********************************************************************/
/** MTC Senden  

	MTC Sende-Variablen:
	
	seqi->snd_mtc			Flag ob MTC Senden an.
	seqi->snd_mtc_frames	Sende-Typ (0-4 entspricht 24-100 Frames)
	seqi->mtc_type			Sende-Typ in Bits zum "einodern" in die 
							Quarter-Frame Message.
	seqi->ms_frames  		Anzahl der Millisekunden pro Frame
	seqi->ms_quarter		Anzahl der Millisekunden pro Quarter-Frame
	mtc_count				Z„hler von 0-7 da 8 Quarter-Frames 
							eine vollst„ndige Message machen. 
	mtc_out_time			Letzte gesendete Zeit, da der Sequenzer 
							die Routine viel ”fter anspringt als 
							n”tig. 	
	stimh, stimm, 
	stims, stimf  			Hilfs-Variablen zur Zerlegung 
							der Millisekunden. 

**/
/********************************************************************/
/********************************************************************/

/********************************************************************/
/** Init Send MTC 
	Wird aufgerufen wenn ein Objekt gestartet wird. 
	ms_clock ist die aktuelle Ms-Position des Objekts, 
	wird aber im Moment nicht ben”tigt. 
**/
/********************************************************************/
void SoStartSendingMtc( SoPtr any_so, LONG ms_clock )
{

if ( !seqi->sync )  return;

/** Test ob MTC senden an **/
if ( !seqi->snd_mtc )  return;

/** Test ob MTC an das gestartete Objekt gekoppelt **/
if ( seqi->remote_so != any_so ) return;

/** Local Init **/  
mtc_count		= 0;
mtc_out_time	= -100;

}

/********************************************************************/
/** Send MidiTimeCode 
	Wird nur dann angesprungen, wenn auch wirklich etwas zu senden ist. 
	Details sind aus der Quarter-Frame Definition zu entnehmen. 
	Siehe auch Delight Sync-Dialogbox: 
	Auf bis zu 9 Out-Ports kann gleichzeitig gesendet werden. 
**/
/********************************************************************/
void send_mtc( void )
{
INT			flag, bit, q;
LONG		lang, clock;
MidiEvPtr	e, ce;

if ( !seqi->sync )  return;

/** Kein Port an, siehe Delight Sync-Dialogbox **/
if ( seqi->snd_mtc_ports == 0 )
  return;

/** QuarterFrame-Event kreiern **/
e = SmRltNewEv( typeQuarterFrame );

/** Speicher voll **/
if ( !e ) return;

/** Zeit mitz„hlen **/
mtc_out_time += seqi->ms_quarter;

/** Nummer der Message ( 0-7 ) **/
Pitch( e ) = mtc_count;

/** Werte eintragen, siehe Defintion der 8 Quarter-Frame Messages **/ 
switch( mtc_count )
  {
  case 0:
    /** Alle Berechnungen durchfhren **/
	
    clock = mtc_out_time;
	if ( seqi->snd_sign_offset )	
	  clock -= seqi->snd_mtc_offset;
	else  
	  clock += seqi->snd_mtc_offset;
	  
	stimh	= (INT)( clock / 3600000L );
	lang	= clock % 3600000L;
	stimm	= (INT)( lang / 60000L );
	lang	= lang % 60000L;
	stimf	= (INT)( lang % 1000L ) / seqi->ms_frames;
	stims	= (INT)( lang / 1000L );
	
    Vel( e ) = ( stimf & 15 );
    mtc_count = 1;
    break;
    
  case 1:
    Vel( e ) = (( stimf & 16 ) >> 4 );
    mtc_count = 2;
    break;
	
  case 2:
    Vel( e ) = ( stims & 15 );
    mtc_count = 3;
    break;
	
  case 3:
    Vel( e ) = (( stims & 48 ) >> 4 );
    mtc_count = 4;
    break;
	
  case 4:
    Vel( e ) = ( stimm & 15 );
    mtc_count = 5;
    break;
	
  case 5:
    Vel( e ) = (( stimm & 48 ) >> 4 );
    mtc_count = 6;
    break;
	
  case 6:
    Vel( e ) = ( stimh & 15 );
    mtc_count = 7;
    break;
	
  case 7:
    Vel( e )  = (( stimh & 16 ) >> 4 );
    Vel( e ) |= (INT)seqi->mtc_type;
    mtc_count = 0;
    break;
  }

/** Quarter Frames auf alle angew„hlten Ports 
	( aužer dem ersten angew„hlten ) schicken. 
**/
bit  = 1;
flag = 0;
for ( q=0; q<9; q++ )
  {
  if ( bit & seqi->snd_mtc_ports )
    {    
    if ( flag )
      {
      ce = SmRltCopyEv( e );
      if ( ce )
        MidiSendIm( seqi->refnum, ce );
      }

    Port( e ) = q;
    flag = 1;
    }
  
  bit <<= 1;  
  }

/** Quarter Frame auf den ersten angew„hlten Kanal schicken **/
MidiSendIm( seqi->refnum, e );

}

/********************************************************************/
/** Senden synchronisieren. 
	Wird vom Sequenzer jede Millisekunde angesprungen. 
	Hier wird getestet ob etwas gesendet werden soll. 
	Wenn der Sequenzer gespult wurde, wird automatisch wieder 
	die richtige Sync-Zeit eingeklinkt. 
**/
/********************************************************************/
void sync_send_mtc( SoPtr any_so, LONG ms_clock )
{

/** Test ob Sequenzer gespult oder frisch gestartet wurde **/
if ( ms_clock >= mtc_out_time + 100L || ms_clock <= mtc_out_time - 100L )
  {
  /** (Re)Initialisieren der Sende-Zeit **/  
  mtc_count	   = 0;
  mtc_out_time = ms_clock;
  } 
  
/** Sende-Zeit ist eingetreten **/
if ( ms_clock >= mtc_out_time )
  
  /** Send **/
  send_mtc();  
  
}


/********************************************************************/
/********************************************************************/
/** MTC empfangen
	MTC Empfangs-Variablen:

	seqi->rcv_mtc			Flag ob MTC empfangen an.
	seqi->mtc_sync			Flag ob die Synchronisation 
							aus, eingelockt oder ob in Planung. 
	seqi->mtc_time			Aktuelle empfangene Zeit, korrigiert um 
							das bliche Delay von zwei Frames. 
	seqi->rcv_mtc_offset	Receive Offset ( auch negative Werte )
	mtc_lock				Flag ob in eine 8-er Sequenz von 
							Quarter-Frames eingelockt. 
	mtc_timer				Aktuelle empfangene Zeit. 
	mtc_dela[]				Zwei Frames Delay-Zeit.
	
**/
/********************************************************************/
/********************************************************************/

/********************************************************************/
/** Receive MidiTimeCode 
	Wird immer von der "receive_alarm" Routine angesprungen, 
	wenn ein Quarter Frame emfangen wurde. 
**/
/********************************************************************/
INT receive_mtc( SmPtr e, INT type, INT pitch, INT vel )
{
LONG help, code_type;

#ifndef RCV_MTC_BY_MIDI_SHARE

if ( !seqi->sync )  return 1;

/** Receive MidiTimeCode **/
if ( seqi->rcv_mtc == NO_RCV_MTC_SYNC )
  return 1;

/** Die erste Quarter Message **/
if ( !pitch )
  mtc_lock = 1;

/** Quarter-Wert zuweisen **/
mtc_val[ pitch ] = vel;

/** Die letzte Quarter Message **/
if ( !(mtc_lock == 1 && pitch == 7) )
  return 1;

/** Code-Type **/
code_type = mtc_val[ 7 ] & 6;

/** Frames **/
mtc_timer  = (LONG)mtc_val[ 0 ];
mtc_timer += (LONG)(( mtc_val[ 1 ] & 1 ) << 4 );
mtc_timer *= 1000L;
mtc_timer /= (mtc_fr[ code_type ]);

/** Sekunden **/
help  = (LONG)mtc_val[ 2 ];
help += (LONG)(( mtc_val[ 3 ] & 3 ) << 4 );
help *= 1000L;
mtc_timer += help;

/** Minuten **/
help  = (LONG)mtc_val[4];
help += (LONG)(( mtc_val[ 5 ] & 3 ) << 4 );
help *= 60000L;
mtc_timer += help;

/** Stunden **/
help  = (LONG)mtc_val[6];
help += (LONG)(( mtc_val[ 7 ] & 1 ) << 4 );
if ( help > 20 ) 
  mtc_timer = 0L;
else
  {  
  help *= 360000L;
  mtc_timer += help;
  }

seqi->mtc_time = mtc_timer + mtc_dela[ code_type ];
seqi->mtc_time -= seqi->rcv_mtc_offset;

mtc_lock = 0;

/** Test ob schon irgendetwas von aužen Synchronisiert wird **/
if ( seqi->mtc_sync == NO_RCV_MTC_SYNC )
  {
  /** Test ob Sequenzer spielt **/
  if ( !seqi->play )
    {
    if ( seqi->mtc_time >= 0L ) 
      {
	  if ( seqi->mmc_is_wind == TRUE )
	    goto ende;
	  
      seqi->mtc_sync = TRY_RCV_MTC_SYNC;
    
      /** Non-Realtime Message den Sequenzer zu starten **/
      if ( seqi->mtc_time < 100L )
   	    SeSetRltAction( (LONG)SoRemote, (LONG)Z_START, 0L, 0L );
   	  else  
   	    SeSetRltAction( (LONG)SoRemote, (LONG)Z_CONT, 0L, 0L );
   	  }
   	}
  }  

#endif RCV_MTC_BY_MIDI_SHARE

ende:

return 1;
}

/********************************************************************/
/** Empfangen synchronisieren.
	Wird jede Millisekunde vom Sequenzer aufgerufen. 
	Da der Sequenzer schneller oder langsamer sein kann, 
	oder sogar gespult oder gecycelt werden kann, 
	muž immer das delay in Millisekunden ermittelt werden, 
	sobald ein neues Quarter Frame eingetroffen ist, 
	um den Abstand zu korrigieren. 
	
	ms_clock		ist die aktuelle interne Zeit
	delay			das bliche Aufruf-Delay von 1 Millisekunde
**/
/********************************************************************/
LONG sync_receive_mtc( SoPtr any_so, LONG ms_clock, LONG delay )
{

/** Nun kommen zwei verschiedene M”glichkeiten der Synchronisation:
	Die eigene Routine und die Routine die die neuen M”glichkeiten 
	von MidiShare ausnutzt. 
**/

/***************************/
/** Die MidiShare Routine, die nicht geht **/
/***************************/
#ifdef RCV_MTC_BY_MIDI_SHARE

seqi->mtc_time  = MidiGetExtTime();
seqi->mtc_time += 160L;
seqi->mtc_time -= seqi->rcv_mtc_offset;

if ( seqi->mtc_time < 0L ) 
  seqi->mtc_time = 0L;
delay 	       = seqi->mtc_time - ms_clock;


/***************************/
/** Die eigene Routine    **/
/***************************/
#else 

/** Ist der Abstand zwischen interner und externer Zeit zu grož, 
	dann wird der Sequenzer gestoppt, 
	da wahrscheinlich auch das externe Signal gestoppt hat. 
**/
if ( (seqi->mtc_time + 500L)  < ms_clock ||
	 (seqi->mtc_time - 1000L) > ms_clock )
  {	 
  seqi->mtc_sync = NO_RCV_MTC_SYNC;
  SeSetRltAction( (LONG)SoRemote, (LONG)Z_STOP, 0L, 0L );
  }

/** Test ob eine neue Quarter-Frame Message erhalten **/ 
if ( old_mtc_time != seqi->mtc_time )
  {
  /** Differenz zwischen interner und externer Zeit berechnen **/
  delay 	   = seqi->mtc_time - ms_clock;
  old_mtc_time = seqi->mtc_time;
  }  
  
#endif RCV_MTC_BY_MIDI_SHARE


return delay;
}


/********************************************************************/
/** Init Receive MTC 
	Wird aufgerufen, wenn der Sequenzer gestartet wird. 
	Sync-Flag auf an, wenn in der receive_mtv Routine 
	bereits ein Qaurter-Frame empfanegn wurde (TRY_RCV_MTC_SYNC). 
**/
/********************************************************************/
void SoStartReceivingMtc( SoPtr any_so )
{

if ( seqi->remote_so != any_so ) return;
 
if ( seqi->mtc_sync == TRY_RCV_MTC_SYNC )
  seqi->mtc_sync = RCV_MTC_SYNC;

old_mtc_time = -1L;

}

/********************************************************************/
/** Stop Receive MTC 
	Wird aufgerufen, wenn der Sequenzer gestartet wird. 
	Sync-Flag auf aus.
**/
/********************************************************************/
void SoStopReceivingMtc( SoPtr any_so )
{

if ( seqi->remote_so != any_so ) return;
  
seqi->mtc_sync = NO_RCV_MTC_SYNC;

}

/********************************************************************/
/********************************************************************/
/** Sonstige MTC-Hilfs-Routinen
**/
/********************************************************************/
/********************************************************************/

/********************************************************************/
/** Synchronisations-Modus setzen. 
	Nur vonn”ten, wenn die neuen MidiShare-Features angesprochen werden.
**/
/********************************************************************/
INT SgSetSyncMode( void )
{

/*

/** Wenn MTC empfangen werden soll **/
if ( seqi->rcv_mtc )
  MidiSetSyncMode( MIDISyncExternal | MIDISyncAnyPort );
else  
  MidiSetSyncMode( MIDISyncInternal );

*/

return 0;
}

/********************************************************************/
/** Frames setzen.
	Wird bei jeder Žnderung der Frame-Art aufgerufen, 
	um die internen Variablen zu setzen. 
**/
/********************************************************************/
INT SgSetMtcFrames( void )
{

/** Anzahl der Millisekunden pro Frame  **/
seqi->ms_frames = ( 1000 / frameval[ seqi->snd_mtc_frames ] );

/** Quarter-Frames werden 4 mal pro Frame gesendet **/
seqi->ms_quarter = seqi->ms_frames / 4;

switch( seqi->snd_mtc_frames )
  {
  case 0:
    seqi->mtc_type = 0;
    break;
    
  case 1:
    seqi->mtc_type = 2;
    break;
    
  case 2:
    seqi->mtc_type = 4;
    break;

  case 3:
    seqi->mtc_type = 6;
    break;
        
  case 4:
    seqi->mtc_type = 0;
    break;
  }

return 0;
}

/********************************************************************/
/** MTC init. 
	Wird beim Programmstart aufgerufen.
**/
/********************************************************************/
void init_mtc( void )
{

seqi->mtc_time	= 0L;
seqi->mtc_sync	= NO_RCV_MTC_SYNC;
seqi->multi_sync_flag = 0;
SgSetMtcFrames();


}


