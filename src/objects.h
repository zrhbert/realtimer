/*****************************************************************************/
/*                                                                           */
/* Modul: OBJECTS.H                                                          */
/* Datum: 29/11/94                                                           */
/*                                                                           */
/* Objekt- und Klassen-Definitionen                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
29.11.94
- window Klassen ausgebaut
14.07.94
- tra_module eingebaut
- CLASS_GMI eingebaut
- info_path eingebaut
- NUM_MODULE_TYPES und NUM_CLASSES eingebaut
- Module() und Window() eingebaut
27.08.93
- CLASS_CMO eingebaut
25.07.93
- wi_finished eingebaut
- create_window_obj eingebaut
- RTMCLASSP help umgebaut
- FILE_xxx Flags und module->xxx_status eingebaut 
- Setup-Class Definitionen eingebaut
- RTM-Window-Class auf > 200 gesetzt um Konflikte zu vermeiden
17.04.93
- Reihenfolge der Modul-Nummern geÑndert
- CLASS_MSH eingefÅgt
*****************************************************************************/

#ifndef __OBJECTS__
#define __OBJECTS__
#define XRSC_CREATE 1                    /* X-Resource-File im Code */

/****** DEFINES **************************************************************/

#define 	WINDOW_INITWIN_OBJ \
		/* Set window initialisation size */ \
		menu_height = (menu != NULL) ? gl_hattr : 0; \
		window->scroll.x  = winit.x ? winit.x : INITX; \
		window->scroll.y  = winit.y ? winit.y + odd (menu_height) : INITY + odd (menu_height) ; \
		window->scroll.w  = winit.w ? winit.w : INITW; \
		window->scroll.h  = winit.h ? winit.h : INITH;

#define 	WINDOW_INITOBJ_OBJ \
		/* Set window initialisation size for dialogs */ \
		menu_height = (menu != NULL) ? gl_hattr : 0; \
		window->scroll.x  = winit.x ? winit.x : INITX; \
		window->scroll.y  = winit.y ? winit.y + odd (menu_height) : INITY + odd (menu_height) ; \
		window->scroll.w  = winit.w ? winit.w : obj->ob_width; \
		window->scroll.h  = winit.h ? winit.h : obj->ob_height;

/* Angabe, wo sich die Setups befinden sollen */
enum SETUPS_LOCATION	{SETUPS_INTERN, SETUPS_EXTERN};

/* Flags fÅr besondere ZustÑnde in module->status */
#define FLAG_IMPORTING 	0x0001
#define FLAG_EXPORTING 	0x0002

/* Flags fÅr Datei-ZustÑnde z.B. module->file_status */
#define FILE_OPENED 		0x0001

/* nur zur Erinnerung, keine Funktion */
#define	PRIVATE	LOCAL
#define	PUBLIC	LOCAL

/****** TYPES ****************************************************************/

typedef struct edit
{
	struct setup	*setup;		/* Zeiger auf eine Setup-Struktur des Moduls */
	LONG				number;		/* Nummer aus der das Setup stammt */
	BOOLEAN			modified;	/* Flag, ob modifiziert wurde */
} EDIT;

typedef	struct edit *ED_P;

typedef struct rtmclass *RTMCLASSP;

typedef struct rtmclass
{
	/* message ist erste Funktion, fÅr KompatibilitÑt mit dispobj */
	VOID		(* message)		(RTMCLASSP module, WORD type, VOID *message);	/* Empfang einer Nachricht */
	STRING	object_name;
	INT		class_number;
	INT		handle;
	OBJECT	*icon;				/* Icon fÅr Deskrtmtop und Manager */	
	WORD		icon_position;		/* Vorgabewert fÅr Icon-Reihenfolge */
	WORD		icon_number;		/* Referenznummer, wird bei Init vergeben */
	STRING	menu_text;			/* MenÅzeile fÅr HauptmenÅ */
	WORD		menu_title;			/* Vorgabetitel */
	WORD		menu_position;		/* Vorgabeposition */
	WORD		menu_item;			/* Referenznummer, wird bei Init vergeben */
	WORD		multiple;			/* Anzahl maximal offener Fenster */
	
	struct window*	(* crt)	_((OBJECT *obj, OBJECT *menu, WORD icon));
	BOOLEAN	(* open) 		_((WORD icon));
	BOOLEAN	(* info) 		_((struct window* window, WORD icon));
	BOOLEAN 	(* icons)		_((WORD src_obj, WORD dest_obj));
	
	BOOLEAN	(* init)			_((VOID));
	BOOLEAN	(* term)			_((VOID));

	/* ZusÑtzlich zu GEMCLASS */

	LONG		priority;			/* PrioritÑt, fÅr Multitasking */
	INT		object_type;		/* INPUT, CALC, o. Ñ. */
	
	struct puf_inf	*(* apply)		_((RTMCLASSP module, struct puf_inf *koor));
	VOID		(* reset)		_((RTMCLASSP module));	/* ZurÅcksetzen von Werten */
	VOID		(* precalc)		_((RTMCLASSP module));	/* Vorausberechnung */
	BOOLEAN	(* help)			_((RTMCLASSP module));
	BOOLEAN	(* test)			_((RTMCLASSP module, WORD action));
	BOOLEAN	(* load)			_((RTMCLASSP module, STR128 filename, BOOLEAN fileselect));	/* Laden von Setups */
	BOOLEAN	(* save)			_((RTMCLASSP module, STR128 filename, BOOLEAN fileselect));
	RTMCLASSP (*create)		_((VOID));
	VOID		(*destroy)		_((RTMCLASSP module));

	/* Setup-Daten */
	struct window*	window;		/* evtl. zugehîriges Fenster */
	BOOL		start_open;			/* Flag fÅr Start mit offenem fenster */
	LONG		special;				/* fÅr andere Verwendung */
	WORD		flags;				/* bes. ZustÑnde, z. B. Imort */
	struct setup	*setups;		/* Zeiger auf Speicherbereich im RAM */
	struct setup 	*standard;	/* Standard-Einstellung */
	ED_P		actual;				/* Aktuelle Einstellung */
	ED_P		edited;				/* Editierte Einstellung */
	LONG		setup_length;		/* sizeof(SETUP) */
	BOOLEAN	location;			/* Setupverwaltung im RAM / auf Platte */
	LONG		max_setups;				/* Max. Setups im RAM */
	BOOLEAN	ram_modified;		/* Daten im RAM geÑndert */
	LONG		file_max;			/* Anzahl Setups in der Datei */
	STR128	file_name;			/* Name der Setup-Datei */
	FILE		*file_pointer;		/* Zeiger auf Setup-Datei */
	LONG		file_header_len;	/* LÑnge des Vorspanns in der Datei */
	STR128	file_version;		/* Datei-Version */
	STRING	file_extension;	/* z.B. "LFO" */
	WORD		file_status;		/* z. B. opened */
	STR128	info_name;			/* Name der Info-Datei */
	STR128	import_name;		/* Name der Datei aus der importiert wird */
	FILE		*import_pointer;	/* Zeiger auf Import-Datei */
	WORD		import_status;		/* z. B. opened */
	STR128	export_name;		/* Name der Datei in die exportiert wird */
	FILE		*export_pointer;	/* Zeiger auf Export-Datei */
	WORD		export_status;		/* z. B. opened */
	struct status	*status;				/* Zeiger auf interne Status-Werte */
	struct status 	*stat_alt;			/* Zeiger auf alte interne Status-Werte */
	/* Funktionen um Setups zu verwalten */
	BOOLEAN	(*import)		_((RTMCLASSP module, STR128 filename, BOOLEAN fileselect));	/* ASCII Import von Setups */
	BOOLEAN	(*export)		_((RTMCLASSP module, STR128 filename, BOOLEAN fileselect));	/* ASCII Export von Setups */
	VOID		(*get_dbox)		_((RTMCLASSP module));		/* DBox -> ed */
	VOID		(*set_dbox)		_((RTMCLASSP module));		/* ed -> DBox */
	VOID		(*get_edit)		_((RTMCLASSP module));		/* ed -> akt */
	VOID		(*set_edit)		_((RTMCLASSP module));		/* akt-> ed  */
	BOOLEAN	(*get_setnr)	_((RTMCLASSP module, LONG setupnr));		/* akt -> set */
	BOOLEAN	(*set_setnr)	_((RTMCLASSP module, LONG setupnr));		/* set -> akt */
	/* Funktionen fÅr DBox Bedienung */
	VOID 		(*set_nr) 			_((struct window* window, LONG setupnr));
	VOID 		(*set_store) 		_((struct window* window, LONG setupnr));
	VOID 		(*set_recall) 		_((struct window* window, LONG setupnr));
	VOID 		(*set_ok) 			_((struct window* window));
	VOID 		(*set_cancel) 		_((struct window* window));
	VOID 		(*set_standard) 	_((struct window* window));
	VOID		(*send_messages)	_((RTMCLASSP module));		/* akt Werte mitteilen */
} RTMCLASS;

/* Modul- und Window-Pointer holen ohne Bus Error */
#define Module(window)	(window ? (RTMCLASSP)window->module : (RTMCLASSP) NULL)
#define Window(module)  (module ? (module->window) : ((struct window*) NULL))
#define Status(window)	Module(window)->status
#define Akt(window)		Module(window)->actual->setup
#define Ed(window)		Module(window)->edited->setup


/* Pointer to avoid RTMCLASSP (*create) (VOID) parameter-declaration */
typedef RTMCLASSP  (CreateFn) (VOID);

/****** VARIABLES ************************************************************/
GLOBAL WORD    rtmtop;                    /* Anzahl der Module */
GLOBAL WORD    max_rtmmodules;            /* Maximale Anzahl Module */
GLOBAL WORD    nortmmodules;              /* Fehler fÅr "Kein Modul mehr" */
GLOBAL SET     used_rtmmodules;           /* Slots der belegten Module */
GLOBAL RTMCLASSP *rtmmodules;					/* Modulkeller */
GLOBAL RTMCLASSP rtmmrec;						/* Speicher fÅr Modulzeiger */
GLOBAL STR128  setup_path;						/* Pfad der Setups */
GLOBAL STR128  import_path;					/* Pfad der Import-Dateien */
GLOBAL STR128  export_path;					/* Pfad der Export-Dateien */
GLOBAL STR128  sysdata_path;					/* Pfad der Systemdaten */
GLOBAL STR128  midishare_path;				/* Pfad fÅr Midishare */
GLOBAL STR128  info_path;						/* Pfad fÅr Info-Dateien */
GLOBAL RECT		winit;							/* Init size for next window */

/* Spezielle Modulpointer */
GLOBAL RTMCLASSP man_module;			/* Zeiger auf MAN-Modul */
GLOBAL RTMCLASSP puf_module;			/* Zeiger auf PUF-Modul */
GLOBAL RTMCLASSP tra_module;			/* Zeiger auf TRA-Modul */
GLOBAL RTMCLASSP var_module;			/* Zeiger auf VAR-Modul */
GLOBAL RTMCLASSP lfo_module;			/* Zeiger auf LFO-Modul */
GLOBAL RTMCLASSP mtr_module;			/* Zeiger auf MTR-Modul */
GLOBAL RTMCLASSP ed4_module;			/* Zeiger auf PUF-Modul */

/****** FUNCTIONS ************************************************************/

GLOBAL RTMCLASSP create_module (CHAR *module_name, WORD instance_count);
GLOBAL VOID			delete_module 		_((RTMCLASSP module));
GLOBAL BOOLEAN		term_modules 		_((VOID));

GLOBAL VOID 		get_edit_obj		_((RTMCLASSP module));
GLOBAL VOID			set_edit_obj		_((RTMCLASSP module));
GLOBAL BOOLEAN		get_setnr_obj		_((RTMCLASSP module, LONG setupnr));
GLOBAL BOOLEAN		set_setnr_obj		_((RTMCLASSP module, LONG setupnr));
GLOBAL VOID 		set_nr_obj 			_((struct window* window, LONG setupnr));
GLOBAL VOID 		set_store_obj 		_((struct window* window, LONG setupnr));
GLOBAL VOID 		set_recall_obj 	_((struct window* window, LONG setupnr));
GLOBAL VOID 		set_ok_obj 			_((struct window* window));
GLOBAL VOID 		set_cancel_obj 	_((struct window* window));
GLOBAL VOID 		set_standard_obj 	_((struct window* window));

GLOBAL BOOLEAN		icons_obj	 		_((WORD src_obj, WORD dest_obj));
/*
GLOBAL struct window*		crt_obj  			_((OBJECT *obj, OBJECT *menu, WORD icon));
GLOBAL BOOLEAN		open_obj	 			_((WORD icon));
GLOBAL BOOLEAN		info_obj				_((struct window* window, WORD icon));
*/
GLOBAL BOOLEAN 	showhelp_obj		_((struct window* window, WORD icon));
GLOBAL BOOLEAN 	help_obj				_((RTMCLASSP module));
GLOBAL BOOLEAN		load_obj				_((RTMCLASSP module, STR128 filename, BOOLEAN fileselect));
GLOBAL BOOLEAN		save_obj				_((RTMCLASSP module, STR128 filename, BOOLEAN fileselect));
GLOBAL BOOLEAN		test_obj				_((RTMCLASSP module, WORD action));
GLOBAL RTMCLASSP	create_obj			_((VOID));
GLOBAL VOID			destroy_obj			_((RTMCLASSP module));
GLOBAL WORD			hndl_alert_obj		_((RTMCLASSP module, WORD alert_id));

GLOBAL VOID    update_menu_obj _((struct window* window));
GLOBAL VOID    handle_menu_obj _((struct window* window, WORD title, WORD item));
GLOBAL VOID    box_obj         _((struct window* window, BOOLEAN grow));
GLOBAL BOOLEAN wi_test_obj     _((struct window* window, WORD action));
GLOBAL VOID    wi_open_obj     _((struct window* window));
GLOBAL VOID    wi_close_obj    _((struct window* window));
GLOBAL VOID    wi_delete_obj   _((struct window* window));
GLOBAL VOID    wi_draw_obj     _((struct window* window));
GLOBAL VOID    wi_finished_obj _((struct window* window));
GLOBAL VOID    wi_arrow_obj    _((struct window* window, WORD dir, LONG oldpos, LONG newpos));
GLOBAL VOID    wi_snap_obj     _((struct window* window, RECT *new, WORD mode));
GLOBAL VOID    wi_objop_obj    _((struct window* window, SET objs, WORD action));
GLOBAL WORD    wi_drag_obj     _((struct window* src_window, WORD src_obj, struct window* dest_window, WORD dest_obj));
GLOBAL VOID    wi_click_obj    _((struct window* window, MKINFO *mk));
GLOBAL VOID    wi_unclick_obj  _((struct window* window));
GLOBAL BOOLEAN wi_key_obj      _((struct window* window, MKINFO *mk));
GLOBAL VOID    wi_timer_obj    _((struct window* window));
GLOBAL VOID    wi_top_obj      _((struct window* window));
GLOBAL VOID    wi_untop_obj    _((struct window* window));
GLOBAL VOID    wi_edit_obj     _((struct window* window, WORD action));

GLOBAL WORD		find_classlot		_((INT rtmclass));
GLOBAL struct window*	create_window_obj _((UWORD kind, WORD class));

/****** Standard-Prototypes ************************************************************/
/*	GEMCLASS-Funktionen */
PUBLIC BOOLEAN icons_mod 		_((WORD src_obj, WORD dest_obj));
PUBLIC struct window* crt_mod   		_((OBJECT *obj, OBJECT *menu, WORD icon));
PUBLIC BOOLEAN open_mod 		_((WORD icon));
PUBLIC BOOLEAN info_mod			_((struct window* window, WORD icon));
PUBLIC BOOLEAN help_mod			_((struct window* window, WORD icon));
PUBLIC BOOLEAN	load_mod			_((RTMCLASSP module, STR128 filename, BOOLEAN fileselect));
PUBLIC BOOLEAN	save_mod			_((RTMCLASSP module, STR128 filename, BOOLEAN fileselect));
PUBLIC BOOLEAN	import_mod		_((RTMCLASSP module, STR128 filename, BOOLEAN fileselect));

PUBLIC BOOLEAN term_mod  		_((VOID));

/*	RTMCLASS-Funktionen */
PUBLIC struct puf_inf 	*apply			_((RTMCLASSP module, struct puf_inf *event));
PUBLIC VOID			precalc			_((RTMCLASSP module));
PUBLIC VOID			message			_((RTMCLASSP module, WORD type, VOID *message));
PUBLIC RTMCLASSP	create			_((VOID));
PUBLIC VOID			destroy_mod		_((RTMCLASSP module));
PUBLIC BOOLEAN		import			_((RTMCLASSP module, STR128 filename, BOOLEAN fileselect));
PUBLIC VOID			reset				_((RTMCLASSP module));
PUBLIC VOID			get_dbox			_((RTMCLASSP module));
PUBLIC VOID			set_dbox			_((RTMCLASSP module));
PUBLIC VOID			send_messages	_((RTMCLASSP module));

/* WINDOW-Funktionen */

PRIVATE VOID    update_menu_mod _((struct window* window));
PRIVATE VOID    handle_menu_mod _((struct window* window, WORD title, WORD item));
PRIVATE BOOLEAN wi_test_mod     _((struct window* window, WORD action));
PRIVATE VOID    wi_open_mod     _((struct window* window));
PRIVATE VOID    wi_close_mod    _((struct window* window));
PRIVATE VOID    wi_delete_mod   _((struct window* window));
PRIVATE VOID    wi_draw_mod     _((struct window* window));
PRIVATE VOID    wi_finished_mod _((struct window* window));
PRIVATE VOID    wi_arrow_mod    _((struct window* window, WORD dir, LONG oldpos, LONG newpos));
PRIVATE VOID    wi_snap_mod     _((struct window* window, RECT *new, WORD mode));
PRIVATE VOID    wi_objop_mod    _((struct window* window, SET objs, WORD action));
PRIVATE WORD    wi_drag_mod     _((struct window* src_window, WORD src_obj, struct window* dest_window, WORD dest_obj));
PRIVATE VOID    wi_click_mod    _((struct window* window, MKINFO *mk));
PRIVATE VOID    wi_unclick_mod  _((struct window* window));
PRIVATE BOOLEAN wi_key_mod      _((struct window* window, MKINFO *mk));
PRIVATE VOID    wi_timer_mod    _((struct window* window));
PRIVATE VOID    wi_top_mod      _((struct window* window));
PRIVATE VOID    wi_untop_mod    _((struct window* window));
PRIVATE VOID    wi_edit_mod     _((struct window* window, WORD action));

PRIVATE VOID    update_menu _((struct window* window));
PRIVATE VOID    handle_menu _((struct window* window, WORD title, WORD item));
PRIVATE BOOLEAN wi_test     _((struct window* window, WORD action));
PRIVATE VOID    wi_open     _((struct window* window));
PRIVATE VOID    wi_close    _((struct window* window));
PRIVATE VOID    wi_delete   _((struct window* window));
PRIVATE VOID    wi_draw     _((struct window* window));
PRIVATE VOID    wi_finished _((struct window* window));
PRIVATE VOID    wi_arrow    _((struct window* window, WORD dir, LONG oldpos, LONG newpos));
PRIVATE VOID    wi_snap     _((struct window* window, RECT *new, WORD mode));
PRIVATE VOID    wi_objop    _((struct window* window, SET objs, WORD action));
PRIVATE WORD    wi_drag     _((struct window* src_window, WORD src_obj, struct window* dest_window, WORD dest_obj));
PRIVATE VOID    wi_click    _((struct window* window, MKINFO *mk));
PRIVATE VOID    wi_unclick  _((struct window* window));
PRIVATE BOOLEAN wi_key      _((struct window* window, MKINFO *mk));
PRIVATE VOID    wi_timer    _((struct window* window));
PRIVATE VOID    wi_top      _((struct window* window));
PRIVATE VOID    wi_untop    _((struct window* window));
PRIVATE VOID    wi_edit     _((struct window* window, WORD action));

GLOBAL BOOL load_info_obj (CONST CHAR *file_name, CHAR *setup_name, LONG *setup_nr, RECT *scroll, BOOL *opened);
GLOBAL BOOL save_info_obj (CONST CHAR *file_name, CONST CHAR *setup_name, CONST LONG setup_nr, CONST RECT *scroll, BOOL opened);
GLOBAL WORD load_create_infos (CreateFn *create, CONST CHAR *type, CONST WORD max_instances);
GLOBAL BOOLEAN open_module_windows (VOID);

#endif /* __OBJECTS__ */
