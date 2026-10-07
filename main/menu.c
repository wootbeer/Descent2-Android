/*
THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.  
COPYRIGHT 1993-1999 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

#pragma off (unreferenced)
static char rcsid[] = "$Id: menu.c 2.152 1997/01/24 18:08:33 jeremy Exp $";
#pragma on (unreferenced)

#ifdef WINDOWS
#include "desw.h"
#endif

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>

#include "pa_enabl.h"                   //$$POLY_ACC

#include "menu.h"
#include "inferno.h"
#include "game.h"
#include "gr.h"
#include "key.h"
#include "iff.h"
#include "mem.h"
#include "error.h"
#include "bm.h"
#include "screens.h"
#include "mono.h"
#include "joy.h"
#include "vecmat.h"
#include "effects.h"
#include "slew.h"
#include "gamemine.h"
#include "gamesave.h"
#include "palette.h"
#include "args.h"
#include "newdemo.h"
#include "timer.h"
#include "sounds.h"
#include "gameseq.h"
#include "text.h"
#include "gamefont.h"
#include "newmenu.h"
#include "network.h"
#include "scores.h"
#include "joydefs.h"
#include "modem.h"
#include "playsave.h"
#include "multi.h"
#include "kconfig.h"
#include "titles.h"
#include "credits.h"
#include "texmap.h"
#include "polyobj.h"
#include "state.h"
#include "mission.h"
#include "songs.h"
#include "config.h"
#include "movie.h"
#include "gamepal.h"
#include "gauges.h"
#include "powerup.h"

#ifdef MACINTOSH
	#include "resource.h"
	#include "isp.h"
	#include <Dialogs.h>
#endif

#ifdef EDITOR
#include "editor\editor.h"
#endif

#if defined(POLY_ACC)
#include "poly_acc.h"
#endif

//char *menu_difficulty_text[] = { "Trainee", "Rookie", "Fighter", "Hotshot", "Insane" };
//char *menu_detail_text[] = { "Lowest", "Low", "Medium", "High", "Highest", "", "Custom..." };

#define MENU_NEW_GAME            0
#define MENU_GAME                               1 
#define MENU_EDITOR                                     2
#define MENU_VIEW_SCORES                        3
#define MENU_QUIT                4
#define MENU_LOAD_GAME                          5
#define MENU_SAVE_GAME                          6
#define MENU_DEMO_PLAY                          8
#define MENU_LOAD_LEVEL                         9
#define MENU_START_IPX_NETGAME                  10
#define MENU_JOIN_IPX_NETGAME                   11
#define MENU_CONFIG                             13
#define MENU_REJOIN_NETGAME                     14
#define MENU_DIFFICULTY                         15
#define MENU_START_SERIAL                       18
#define MENU_HELP                               19
#define MENU_NEW_PLAYER                         20
#define MENU_MULTIPLAYER                        21
#define MENU_STOP_MODEM                         22
#define MENU_SHOW_CREDITS                       23
#define MENU_ORDER_INFO                         24
#define MENU_PLAY_SONG                          25
#define MENU_START_TCP_NETGAME                  26
#define MENU_JOIN_TCP_NETGAME                   27
#define MENU_START_APPLETALK_NETGAME			28
#define MENU_JOIN_APPLETALK_NETGAME				30

//ADD_ITEM("Start netgame...", MENU_START_NETGAME, -1 );
//ADD_ITEM("Send net message...", MENU_SEND_NET_MESSAGE, -1 );

#define ADD_ITEM(t,value,key)  do { m[num_options].type=NM_TYPE_MENU; m[num_options].text=t; menu_choice[num_options]=value;num_options++; } while (0)

extern int last_joy_time;               //last time the joystick was used
#ifndef NDEBUG
extern int Speedtest_on;
#else
#define Speedtest_on 0
#endif

ubyte do_auto_demo = 1;                 // Flag used to enable auto demo starting in main menu.
int Player_default_difficulty; // Last difficulty level chosen by the player
int Auto_leveling_on = 0;
int Guided_in_big_window = 0;
int Menu_draw_copyright = 0;

// Options menu "Invert Y" checkbox. DescentView.java's stick handling has always behaved as
// "invert Y enabled", so this defaults ON (1) to keep existing behavior unchanged; turning
// it off is what flips anything. See gamepad_remap.c's isInvertYEnabled() JNI getter.
int Config_invert_y = 1;

// Android-side hooks -- Descent2/src/main/cpp/motion.c and gamepad_remap.c.
extern void startMotion();
extern void stopMotion();
extern int haveGyroscope();
extern void setRenderScaleIndex(int index);
extern void setAspectRatio43(int enabled);
extern void restartAppForSettingsChange();
extern void openMissionPackPicker(void);
extern void do_remap_gamepad_menu(void);
extern ubyte Config_touch_control_scale;
extern ubyte Config_touch_control_opacity;
extern void touch_control_scale_changed(void);
int EscortHotKeys=1;

// Function Prototypes added after LINTING
void do_option(int select);
void do_detail_level_menu_custon(void);
void do_multi_player_menu(void);
void do_detail_level_menu_custom(void);
void do_new_game_menu(void);

extern void ReorderSecondary();
extern void ReorderPrimary();

//returns the number of demo files on the disk
int newdemo_count_demos();
extern ubyte Version_major,Version_minor;

// ------------------------------------------------------------------------
void autodemo_menu_check(int nitems, newmenu_item * items, int *last_key, int citem )
{
	int curtime;

	nitems = nitems;
	items=items;
	citem = citem;

	//draw copyright message
	if ( Menu_draw_copyright )              {
		int w,h,aw;

		Menu_draw_copyright = 0;
		WINDOS(	dd_gr_set_current_canvas(NULL),
					gr_set_current_canvas(NULL));
		gr_set_curfont(GAME_FONT);
		gr_set_fontcolor(BM_XRGB(6,6,6),-1);

		gr_get_string_size("V2.2", &w, &h, &aw );
	
		WIN(DDGRLOCK(dd_grd_curcanv));
			gr_printf(0x8000,grd_curcanv->cv_bitmap.bm_h-GAME_FONT->ft_h*f2fl(Scale_y)-2,TXT_COPYRIGHT);
			#ifdef MACINTOSH	// print out fix level as well if it exists
				if (Version_fix != 0)
				{
					gr_get_string_size("V2.2.2", &w, &h, &aw );
					gr_printf(grd_curcanv->cv_bitmap.bm_w-w-2,
							  grd_curcanv->cv_bitmap.bm_h-GAME_FONT->ft_h-2,
							  "V%d.%d.%d",
							  Version_major,Version_minor,Version_fix);
				}
				else
				{
					gr_printf(grd_curcanv->cv_bitmap.bm_w-w-2,
							  grd_curcanv->cv_bitmap.bm_h-GAME_FONT->ft_h-2,
							  "V%d.%d",
							  Version_major,Version_minor);
				}
			#else
				gr_printf(grd_curcanv->cv_bitmap.bm_w-w-2*f2fl(Scale_x),grd_curcanv->cv_bitmap.bm_h-GAME_FONT->ft_h*f2fl(Scale_y)-2,"V%d.%d",Version_major,Version_minor);
			#endif

		#ifdef SANTA		//say this is hoard version
		if (HoardEquipped()) {
			gr_set_curfont(MEDIUM2_FONT);
			gr_printf(MenuHires?495:00,MenuHires?88:44,"Vertigo");
		}
		#endif

		WIN(DDGRUNLOCK(dd_grd_curcanv));
	}
	
	// Don't allow them to hit ESC in the main menu.
	if (*last_key==KEY_ESC) *last_key = 0;

#ifdef ANDROID_NDK
	// No idle attract mode (intro movie or demos) on Android -- the menu just stays put.
	do_auto_demo = 0;
#endif
	if ( do_auto_demo )     {
		curtime = timer_get_approx_seconds();
		//if ( ((keyd_time_when_last_pressed+i2f(20)) < curtime) && ((last_joy_time+i2f(20)) < curtime) && (!Speedtest_on)  ) {
		#ifndef MACINTOSH		// for now only!!!!
		if ( ((keyd_time_when_last_pressed+i2f(25)) < curtime) && (!Speedtest_on)  ) {
		#else
		if ( (keyd_time_when_last_pressed+i2f(40)) < curtime ) {
		#endif
			int n_demos;

			n_demos = newdemo_count_demos();

#ifdef ANDROID_NDK
			// The idle "play the intro movie" attract mode isn't supported on Android (movie playback
			// fights with the GL surface and audio after a pause/resume). Only ever start demos.
			if (n_demos < 1) {
				keyd_time_when_last_pressed = curtime;
				return;
			}
#endif
try_again:;

#ifdef ANDROID_NDK
			if (0)
#else
			if ((drand() % (n_demos+1)) == 0)
#endif
			{
				#ifndef SHAREWARE
					#ifdef WINDOWS
					mouse_set_mode(1);				//re-enable centering mode
					HideCursorW();
					#endif
					gr_set_current_canvas(NULL);
					PlayMovie("intro.mve",0);
					songs_play_song(SONG_TITLE,1);
					*last_key = -3; //exit menu to force redraw even if not going to game mode. -3 tells menu system not to restore
					set_screen_mode(SCREEN_MENU);
					#ifdef WINDOWS
					mouse_set_mode(0);				//disenable centering mode
					ShowCursorW();
					#endif
				#endif // end of ifndef shareware
			}
			else {
				WIN(HideCursorW());
				keyd_time_when_last_pressed = curtime;                  // Reset timer so that disk won't thrash if no demos.
				newdemo_start_playback(NULL);           // Randomly pick a file
				if (Newdemo_state == ND_STATE_PLAYBACK) {
					Function_mode = FMODE_GAME;
					*last_key = -3; //exit menu to get into game mode. -3 tells menu system not to restore
				}
				else
					goto try_again;	//keep trying until we get a demo that works
			}
		}
	}
}

//static int First_time = 1;
static int main_menu_choice = 0;

//      -----------------------------------------------------------------------------
//      Create the main menu.
void create_main_menu(newmenu_item *m, int *menu_choice, int *callers_num_options)
{
	int     num_options;

	#ifndef DEMO_ONLY
	num_options = 0;

	ADD_ITEM(TXT_NEW_GAME,MENU_NEW_GAME,KEY_N);

	ADD_ITEM(TXT_LOAD_GAME,MENU_LOAD_GAME,KEY_L);

	ADD_ITEM(TXT_MULTIPLAYER_,MENU_MULTIPLAYER,-1);

	ADD_ITEM(TXT_OPTIONS_, MENU_CONFIG, -1 );
	ADD_ITEM(TXT_CHANGE_PILOTS,MENU_NEW_PLAYER,unused);
	ADD_ITEM(TXT_VIEW_DEMO,MENU_DEMO_PLAY,0);
	ADD_ITEM(TXT_VIEW_SCORES,MENU_VIEW_SCORES,KEY_V);
	#ifdef SHAREWARE
	ADD_ITEM(TXT_ORDERING_INFO,MENU_ORDER_INFO,-1);
	#endif
	ADD_ITEM(TXT_CREDITS,MENU_SHOW_CREDITS,-1);
	#endif
	ADD_ITEM(TXT_QUIT,MENU_QUIT,KEY_Q);

	#ifndef RELEASE
	if (!(Game_mode & GM_MULTI ))   {
		//m[num_options].type=NM_TYPE_TEXT;
		//m[num_options++].text=" Debug options:";

		ADD_ITEM("  Load level...",MENU_LOAD_LEVEL ,KEY_N);
		#ifdef EDITOR
		ADD_ITEM("  Editor", MENU_EDITOR, KEY_E);
		#endif
	}

	//ADD_ITEM( "  Play song", MENU_PLAY_SONG, -1 );
	#endif

	*callers_num_options = num_options;
}

//returns number of item chosen
int DoMenu() 
{
	int menu_choice[25];
	newmenu_item m[25];
	int num_options = 0;

	load_palette(MENU_PALETTE,0,1);		//get correct palette

	if ( Players[Player_num].callsign[0]==0 )       {
		RegisterPlayer();
		return 0;
	}
	
	if ((Game_mode & GM_SERIAL) || (Game_mode & GM_MODEM)) {
		do_option(MENU_START_SERIAL);
		return 0;
	}

	create_main_menu(m, menu_choice, &num_options);

	do {
		keyd_time_when_last_pressed = timer_get_fixed_seconds();                // .. 20 seconds from now!
		if (main_menu_choice < 0 )      main_menu_choice = 0;           
		Menu_draw_copyright = 1;
		main_menu_choice = newmenu_do2( "", NULL, num_options, m, autodemo_menu_check, main_menu_choice, Menu_pcx_name);
		if ( main_menu_choice > -1 ) do_option(menu_choice[main_menu_choice]);
		create_main_menu(m, menu_choice, &num_options); //      may have to change, eg, maybe selected pilot and no save games.
	} while( Function_mode==FMODE_MENU );

//      if (main_menu_choice != -2)
//              do_auto_demo = 0;               // No more auto demos
	if ( Function_mode==FMODE_GAME )        
		gr_palette_fade_out( gr_palette, 32, 0 );

	return main_menu_choice;
}

extern void show_order_form(void);      // John didn't want this in inferno.h so I just externed it.

#ifdef WINDOWS
#undef TXT_SELECT_DEMO
#define TXT_SELECT_DEMO "Select Demo\n<Ctrl-D> or Right-click\nto delete"
#endif

//returns flag, true means quit menu
void do_option ( int select) 
{
	switch (select) {
		case MENU_NEW_GAME:
			do_new_game_menu();
			break;
		case MENU_GAME:
			break;
		case MENU_DEMO_PLAY:
			{ 
				char demo_file[16];
				if (newmenu_get_filename( TXT_SELECT_DEMO, ".\\demos\\*.dem", demo_file, 1 ))     {
					newdemo_start_playback(demo_file);
				}
			}
			break;
		case MENU_LOAD_GAME:
			state_restore_all(0, 0, NULL);
			break;
		#ifdef EDITOR
		case MENU_EDITOR:
			Function_mode = FMODE_EDITOR;
			init_cockpit();
			break;
		#endif
		case MENU_VIEW_SCORES:
			gr_palette_fade_out( gr_palette,32,0 );
			scores_view(-1);
			break;
		#ifdef SHAREWARE
		case MENU_ORDER_INFO:
			show_order_form();
			break;
		#endif
		case MENU_QUIT:
			#ifdef EDITOR
			if (! SafetyCheck()) break;
			#endif
			gr_palette_fade_out( gr_palette,32,0);
			Function_mode = FMODE_EXIT;
			break;
		case MENU_NEW_PLAYER:
			RegisterPlayer();               //1 == allow escape out of menu
			break;

		case MENU_HELP:
			do_show_help();
			break;

		#ifndef RELEASE

		case MENU_PLAY_SONG:    {
				int i;
				char * m[MAX_NUM_SONGS];

				for (i=0;i<Num_songs;i++) {
					m[i] = Songs[i].filename;
				}
				i = newmenu_listbox( "Select Song", Num_songs, m, 1, NULL );

				if ( i > -1 )   {
					songs_play_song( i, 0 );
				}
			}
			break;
		case MENU_LOAD_LEVEL: {
			newmenu_item m;
			char text[10]="";
			int new_level_num;

			m.type=NM_TYPE_INPUT; m.text_len = 10; m.text = text;

			newmenu_do( NULL, "Enter level to load", 1, &m, NULL );

			new_level_num = atoi(m.text);

			if (new_level_num!=0 && new_level_num>=Last_secret_level && new_level_num<=Last_level)  {
				gr_palette_fade_out( gr_palette, 32, 0 );
				StartNewGame(new_level_num);
			}

			break;
		}
		#endif


		case MENU_START_IPX_NETGAME:
			load_mission(0);
			#ifdef MACINTOSH
			Network_game_type = IPX_GAME;
			#endif
//			WIN(ipx_create_read_thread());
			network_start_game();
			break;

		case MENU_JOIN_IPX_NETGAME:
			load_mission(0);
			#ifdef MACINTOSH
			Network_game_type = IPX_GAME;
			#endif
//			WIN(ipx_create_read_thread());
			network_join_game();
			break;

#ifdef MACINTOSH
		case MENU_START_APPLETALK_NETGAME:
			load_mission(0);
			#ifdef MACINTOSH
			Network_game_type = APPLETALK_GAME;
			#endif
			network_start_game();
			break;

		case MENU_JOIN_APPLETALK_NETGAME:
			load_mission(0);
			#ifdef MACINTOSH
			Network_game_type = APPLETALK_GAME;
			#endif
			network_join_game();
			break;
#endif
		
		case MENU_START_TCP_NETGAME:
		case MENU_JOIN_TCP_NETGAME:
			nm_messagebox (TXT_SORRY,1,TXT_OK,"Not available in shareware version!");
			// DoNewIPAddress();
			break;
                  
		case MENU_START_SERIAL:
			com_main_menu();
			break;
		case MENU_MULTIPLAYER:
			do_multi_player_menu();
			break;
		case MENU_CONFIG:
			do_options_menu();
			break;
		case MENU_SHOW_CREDITS:
			gr_palette_fade_out( gr_palette,32,0);
			songs_stop_all();
			credits_show(NULL); 
			break;
		default:
			Error("Unknown option %d in do_option",select);
			break;
	}

}

int do_difficulty_menu()
{
	int s;
	newmenu_item m[5];

	m[0].type=NM_TYPE_MENU; m[0].text=MENU_DIFFICULTY_TEXT(0);
	m[1].type=NM_TYPE_MENU; m[1].text=MENU_DIFFICULTY_TEXT(1);
	m[2].type=NM_TYPE_MENU; m[2].text=MENU_DIFFICULTY_TEXT(2);
	m[3].type=NM_TYPE_MENU; m[3].text=MENU_DIFFICULTY_TEXT(3);
	m[4].type=NM_TYPE_MENU; m[4].text=MENU_DIFFICULTY_TEXT(4);

	s = newmenu_do1( NULL, TXT_DIFFICULTY_LEVEL, NDL, m, NULL, Difficulty_level);

	if (s > -1 )    {
		if (s != Difficulty_level)
		{       
			Player_default_difficulty = s;
			write_player_file();
		}
		Difficulty_level = s;
		mprintf((0, "%s %s %i\n", TXT_DIFFICULTY_LEVEL, TXT_SET_TO, Difficulty_level));
		return 1;
	}
	return 0;
}

int     Max_debris_objects, Max_objects_onscreen_detailed;
int     Max_linear_depth_objects;

byte    Object_complexity=2, Object_detail=2;
byte    Wall_detail=2, Wall_render_depth=2, Debris_amount=2, SoundChannels = 2;

// Index into the Android render-scale presets (1.0x..2.0x). Persisted via config.c.
int Render_scale_index = 0;

// "Force 4:3": 0 = stretch to the device screen, 1 = render into a centered 4:3 region.
// Like render scale this reshapes the render surface, which can only safely change at app
// startup -- see setAspectRatio43() (motion.c) and DescentActivity.java.
int Aspect_ratio_4_3 = 0;

// Set when Render Scale or Force 4:3 actually changes during a Video Options visit; checked
// once on exit (not per slider tick) to trigger restartAppForSettingsChange().
static int Video_settings_dirty = 0;

// In-game FPS counter (main/game.c) as a normal persisted option.
extern int framerate_on;

byte    Render_depths[NUM_DETAIL_LEVELS-1] =                        { 6,  9, 12, 15, 50};
byte    Max_perspective_depths[NUM_DETAIL_LEVELS-1] =               { 1,  2,  3,  5,  8};
byte    Max_linear_depths[NUM_DETAIL_LEVELS-1] =                    { 3,  5,  7, 10, 50};
byte    Max_linear_depths_objects[NUM_DETAIL_LEVELS-1] =            { 1,  2,  3,  7, 20};
byte    Max_debris_objects_list[NUM_DETAIL_LEVELS-1] =              { 2,  4,  7, 10, 15};
byte    Max_objects_onscreen_detailed_list[NUM_DETAIL_LEVELS-1] =   { 2,  4,  7, 10, 15};
byte    Smts_list[NUM_DETAIL_LEVELS-1] =                            { 2,  4,  8, 16, 50};   //      threshold for models to go to lower detail model, gets multiplied by obj->size
byte    Max_sound_channels[NUM_DETAIL_LEVELS-1] =                   { 2,  4,  8, 12, 16};

//      -----------------------------------------------------------------------------
//      Set detail level based stuff.
//      Note: Highest detail level (detail_level == NUM_DETAIL_LEVELS-1) is custom detail level.
void set_detail_level_parameters(int detail_level)
{
	Assert((detail_level >= 0) && (detail_level < NUM_DETAIL_LEVELS));

	if (detail_level < NUM_DETAIL_LEVELS-1) {
		Render_depth = Render_depths[detail_level];
		Max_perspective_depth = Max_perspective_depths[detail_level];
		Max_linear_depth = Max_linear_depths[detail_level];
		Max_linear_depth_objects = Max_linear_depths_objects[detail_level];

		Max_debris_objects = Max_debris_objects_list[detail_level];
		Max_objects_onscreen_detailed = Max_objects_onscreen_detailed_list[detail_level];

		Simple_model_threshhold_scale = Smts_list[detail_level];

		digi_set_max_channels( Max_sound_channels[ detail_level ] );

		//      Set custom menu defaults.
		Object_complexity = detail_level;
		Wall_render_depth = detail_level;
		Object_detail = detail_level;
		Wall_detail = detail_level;
		Debris_amount = detail_level;
		SoundChannels = detail_level;

#if defined(POLY_ACC)

		#ifdef MACINTOSH
			if (detail_level < 2)
			{
				pa_set_filtering(0);
			}
			else if (detail_level < 4)
			{
				pa_set_filtering(1);
			}
			else if (detail_level == 4)
			{
				pa_set_filtering(2);
			}
		#else
			pa_filter_mode = detail_level;
		#endif
#endif

	}
}

//      -----------------------------------------------------------------------------
void do_detail_level_menu(void)
{
	int s;
	newmenu_item m[7];

	m[0].type=NM_TYPE_MENU; m[0].text=MENU_DETAIL_TEXT(0);
	m[1].type=NM_TYPE_MENU; m[1].text=MENU_DETAIL_TEXT(1);
	m[2].type=NM_TYPE_MENU; m[2].text=MENU_DETAIL_TEXT(2);
	m[3].type=NM_TYPE_MENU; m[3].text=MENU_DETAIL_TEXT(3);
	m[4].type=NM_TYPE_MENU; m[4].text=MENU_DETAIL_TEXT(4);
	m[5].type=NM_TYPE_TEXT; m[5].text="";
	m[6].type=NM_TYPE_MENU; m[6].text=MENU_DETAIL_TEXT(5);

	s = newmenu_do1( NULL, TXT_DETAIL_LEVEL , NDL+2, m, NULL, Detail_level);

	if (s > -1 )    {
		switch (s)      {
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
				Detail_level = s;
				mprintf((0, "Detail level set to %i\n", Detail_level));
				set_detail_level_parameters(Detail_level);
				break;
			case 6:
				Detail_level = 5;
				do_detail_level_menu_custom();
				break;
		}
	}

}

//      -----------------------------------------------------------------------------
void do_detail_level_menu_custom_menuset(int nitems, newmenu_item * items, int *last_key, int citem )
{
	int in_title_menu = (Function_mode == FMODE_MENU);
	// Render Scale and Force 4:3 are title-screen-only (see do_detail_level_menu_custom()),
	// so they shift every later item's index by 2 when present, 0 when not.
	int title_extra = in_title_menu ? 2 : 0;

	nitems = nitems;
	*last_key = *last_key;
	citem = citem;

	Object_complexity = items[0].value;
	Object_detail = items[1].value;
	Wall_detail = items[2].value;
	Wall_render_depth = items[3].value;
	Debris_amount = items[4].value;

	if (in_title_menu) {
		if (Render_scale_index != items[5].value) {
			Render_scale_index = items[5].value;
			setRenderScaleIndex(Render_scale_index);
			Video_settings_dirty = 1;
		}
		if (Aspect_ratio_4_3 != items[6].value) {
			Aspect_ratio_4_3 = items[6].value;
			setAspectRatio43(Aspect_ratio_4_3);
			Video_settings_dirty = 1;
		}
	}

	framerate_on = items[5 + title_extra].value;
}

void set_custom_detail_vars(void)
{
	Render_depth = Render_depths[Wall_render_depth];

	Max_perspective_depth = Max_perspective_depths[Wall_detail];
	Max_linear_depth = Max_linear_depths[Wall_detail];

	Max_debris_objects = Max_debris_objects_list[Debris_amount];

	Max_objects_onscreen_detailed = Max_objects_onscreen_detailed_list[Object_complexity];
	Simple_model_threshhold_scale = Smts_list[Object_complexity];
	Max_linear_depth_objects = Max_linear_depths_objects[Object_detail];
}

extern void game_flush_inputs(void);
extern void showRenderBuffer(void);	// render.c -- presents the frame (eglSwapBuffers)
extern void delay(unsigned long time);	// main/kconfig.c -- usleep throttle

// Shown right before the app closes for a settings-triggered restart (see
// do_detail_level_menu_custom()). A flat 2-second timer rather than waiting for input,
// because the always-on analog-stick menu navigation made a reliable "wait for a genuine
// press" check more trouble than it was worth. No canvas save/restore needed: the process
// closes moments after this returns.
static void show_video_restart_message(void)
{
	grs_font *title_font = Gamefonts[GFONT_BIG_1];
	fix close_at;

	gr_set_current_canvas(NULL);
	nm_draw_background(0, 0, grd_curcanv->cv_bitmap.bm_w - 1, grd_curcanv->cv_bitmap.bm_h - 1);

	gr_set_curfont(title_font);
	gr_set_fontcolor(BM_XRGB(31, 31, 31), -1);
	gr_scale_printf(0x8000, grd_curcanv->cv_bitmap.bm_h / 2 - title_font->ft_h * f2fl(Scale_factor),
		Scale_factor, Scale_factor, "Settings changed.");
	gr_scale_printf(0x8000, grd_curcanv->cv_bitmap.bm_h / 2 + title_font->ft_h * f2fl(Scale_factor),
		Scale_factor, Scale_factor, "Restart required.");

#ifdef OGLES
	showRenderBuffer();
#endif

	game_flush_inputs();
	close_at = timer_get_approx_seconds() + i2f(2);
	while (timer_get_approx_seconds() < close_at) {
		delay(10);
	}
}

#define	DL_MAX	10

//      -----------------------------------------------------------------------------

void do_detail_level_menu_custom(void)
{
	int	s=0;
	// Render scale and Force 4:3 change the shape/size of the render surface, which this
	// engine only sets up once at startup, so they're only offered from the title screen.
	int in_title_menu = (Function_mode == FMODE_MENU);
	int title_extra = in_title_menu ? 2 : 0;
	newmenu_item m[DL_MAX];

	// Cleared on every entry so only a change made during THIS visit triggers a restart.
	Video_settings_dirty = 0;

	do {
		m[0].type = NM_TYPE_SLIDER;
		m[0].text = TXT_OBJ_COMPLEXITY;
		m[0].value = Object_complexity;
		m[0].min_value = 0;
		m[0].max_value = NDL-1;

		m[1].type = NM_TYPE_SLIDER;
		m[1].text = TXT_OBJ_DETAIL;
		m[1].value = Object_detail;
		m[1].min_value = 0;
		m[1].max_value = NDL-1;

		m[2].type = NM_TYPE_SLIDER;
		m[2].text = TXT_WALL_DETAIL;
		m[2].value = Wall_detail;
		m[2].min_value = 0;
		m[2].max_value = NDL-1;

		m[3].type = NM_TYPE_SLIDER;
		m[3].text = TXT_WALL_RENDER_DEPTH;
		m[3].value = Wall_render_depth;
		m[3].min_value = 0;
		m[3].max_value = NDL-1;

		m[4].type = NM_TYPE_SLIDER;
		m[4].text = TXT_DEBRIS_AMOUNT;
		m[4].value = Debris_amount;
		m[4].min_value = 0;
		m[4].max_value = NDL-1;

		if (in_title_menu) {
			m[5].type = NM_TYPE_SLIDER;
			m[5].text = "Render Scale";
			m[5].value = Render_scale_index;
			m[5].min_value = 0;
			m[5].max_value = 4;

			// Worded "Force" so a player already on a 4:3 device isn't misled: this is only
			// for stretched (e.g. 16:9) devices wanting the pillarboxed original look.
			m[6].type = NM_TYPE_CHECK;
			m[6].text = "Force 4:3";
			m[6].value = Aspect_ratio_4_3;
		}

		m[5 + title_extra].type = NM_TYPE_CHECK;
		m[5 + title_extra].text = "Show FPS Counter";
		m[5 + title_extra].value = framerate_on;

		s = newmenu_do1(NULL, "Video Options", 6 + title_extra, m, do_detail_level_menu_custom_menuset, s);
	} while (s > -1);

	set_custom_detail_vars();

	// Render Scale and/or Force 4:3 changed -- both need a fresh process. Android can't
	// reliably relaunch itself in the foreground, so: save everything, tell the player, close.
	// The next manual launch uses "-quickresume" to skip intro/logos and land on the main menu.
	if (in_title_menu && Video_settings_dirty) {
		// Persist now: normally config is only written when the session ends, which doesn't
		// happen on this path, and the menu would otherwise read stale values next launch.
		WriteConfigFile();

		show_video_restart_message();

		restartAppForSettingsChange();
	}
}

int Default_display_mode=1;
int Current_display_mode=1;

extern int MenuHiresAvailable;

typedef struct {
	int	VGA_mode;
	short	w,h;
	short	render_method;
	short	flags;
} dmi;

dmi display_mode_info[7] = {
		/*#ifdef WINDOWS
			{SM95_320x200x8X, 320, 200, VR_NONE, VRF_ALLOW_COCKPIT},
			{SM95_640x480x8, 640, 480, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT},
			{SM95_640x400x8, 640, 400, VR_NONE, VRF_COMPATIBLE_MENUS }, 
			{SM95_800x600x8, 800, 600, VR_NONE, VRF_COMPATIBLE_MENUS },
			{SM95_1024x768x8, 1024, 768, VR_NONE, VRF_COMPATIBLE_MENUS }, 
		#else
			{SM_320x200C,	 320,	200, VR_NONE, VRF_ALLOW_COCKPIT+VRF_COMPATIBLE_MENUS}, 
			{SM_640x480V,	 640, 480, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT},
			{SM_320x400U,	 320, 400, VR_NONE, VRF_USE_PAGING},
			{SM_640x400V,	 640, 400, VR_NONE, VRF_COMPATIBLE_MENUS}, 
			{SM_800x600V,	 800, 600, VR_NONE, VRF_COMPATIBLE_MENUS}, 
			{SM_1024x768V,	1024,	768, VR_NONE, VRF_COMPATIBLE_MENUS}, 	
			{SM_1280x1024V,1280,1024, VR_NONE, VRF_COMPATIBLE_MENUS}, 
		#endif*/
};
 
WIN(extern int DD_Emulation);


void set_display_mode(int mode)
{
	dmi *dmi;

	if ((Current_display_mode == -1)||(VR_render_mode != VR_NONE))	//special VR mode
		return;								//...don't change

	#if !defined(MACINTOSH) && !defined(WINDOWS)
	if (mode >= 5 && !FindArg("-superhires"))
		mode = 4;
	#endif

	if (!MenuHiresAvailable && (mode != 2))
		mode = 0;

#ifndef WINDOWS
	mode = 1;
	Current_display_mode = mode;
#else
	if (mode == 2) mode = 3;					// 320x400 -> 640x400.
	Current_display_mode = mode;
	if (mode >= 3) mode--;						// Match to Windows dmi.
	if (DDCheckMode(display_mode_info[mode].VGA_mode)) {
		if (Platform_system == WINNT_PLATFORM || DD_Emulation) mode = 1;
		else mode = 0;
		Current_display_mode = mode;
	}
#endif

	dmi = &display_mode_info[mode];

	if (Current_display_mode != -1) {

		game_init_render_buffers(dmi->VGA_mode,dmi->w,dmi->h,dmi->render_method,dmi->flags);
		Default_display_mode = Current_display_mode;
	}

	Screen_mode = -1;		//force screen reset
}

#ifdef MACINTOSH	// use Mac version of do_screen_res_menu

void do_screen_res_menu()
{
	#define N_SCREENRES_ITEMS 6
	
	newmenu_item m[N_SCREENRES_ITEMS];
	int citem, i, n_items, odisplay_mode, result;

	if ((Current_display_mode == -1)||(VR_render_mode != VR_NONE))		//special VR mode
	{				
		nm_messagebox(TXT_SORRY, 1, TXT_OK, 
				"You may not change screen\n"
				"resolution when VR modes enabled.");
		return;
	}

	m[0].type=NM_TYPE_TEXT;	 m[0].value=0; m[0].text="Modes w/ Cockpit:";
	m[1].type=NM_TYPE_RADIO; m[1].value=0; m[1].group=0; m[1].text=" 640x480";
	m[2].type=NM_TYPE_TEXT;	 m[2].value=0; m[2].text="Modes w/o Cockpit:";
	m[3].type=NM_TYPE_RADIO; m[3].value=0; m[3].group=0; m[3].text=" 800x600";
//	m[4].type=NM_TYPE_RADIO; m[4].value=0; m[4].group=0; m[4].text=" 1024x768";
//	m[5].type=NM_TYPE_RADIO; m[5].value=0; m[5].group=0; m[5].text=" 1280x1024";
	n_items = 4;

	odisplay_mode = VGA_current_mode;
	citem = Current_display_mode;
	if (Current_display_mode >= 2)
		citem--;

	if (citem >= n_items)
		citem = n_items-1;

	m[citem].value = 1;

	newmenu_do1( NULL, "Select screen mode", n_items, m, NULL, citem);

	for (i=0;i<n_items;i++)
		if (m[i].value)
			break;
	if (i >= 3)
		i++;

#ifdef SHAREWARE
	if (i > 1)
		nm_messagebox(TXT_SORRY, 1, TXT_OK, 
			"High resolution modes are\n"
			"only available in the\n"
			"Commercial version of Descent 2.");
	return;
#else
	result = vga_check_mode(display_mode_info[i].VGA_mode);
	
	if (result) {
		nm_messagebox(TXT_SORRY, 1, TXT_OK, 
				"Cannot set requested\n"
				"mode on this video card.");
		return;
	}
	
	set_display_mode(i);
	reset_cockpit();
#endif

}

#else	// PC version of do_screen_res_menu is below

void do_screen_res_menu()
{
	#define N_SCREENRES_ITEMS 9
	newmenu_item m[N_SCREENRES_ITEMS];
	int citem;
	int i;
	int n_items;
	int result;

	if ((Current_display_mode == -1)||(VR_render_mode != VR_NONE)) {				//special VR mode
		nm_messagebox(TXT_SORRY, 1, TXT_OK, 
				"You may not change screen\n"
				"resolution when VR modes enabled.");
		return;
	}

	m[0].type=NM_TYPE_TEXT;	 m[0].value=0;    			  m[0].text="Modes w/ Cockpit:";
	
#ifdef WINDOWS
	if (Platform_system == WINNT_PLATFORM || DD_Emulation) {
		m[1].type=NM_TYPE_TEXT; m[1].value=0; m[1].text=" 320x200 N/A";
	} else
#endif
		//NOTE LINK TO ABOVE IF
		m[1].type=NM_TYPE_RADIO; m[1].value=0; m[1].group=0; m[1].text=" 320x200";

	m[2].type=NM_TYPE_RADIO; m[2].value=0; m[2].group=0; m[2].text=" 640x480";
	m[3].type=NM_TYPE_TEXT;	 m[3].value=0;   				  m[3].text="Modes w/o Cockpit:";
#ifdef WINDOWS
	m[4].type=NM_TYPE_RADIO; m[4].value=0; m[4].group=0; m[4].text=" 640x400";
	m[5].type=NM_TYPE_RADIO; m[5].value=0; m[5].group=0; m[5].text=" 800x600";
//	m[6].type=NM_TYPE_RADIO; m[6].value=0; m[6].group=0; m[6].text=" 1024x768";
	n_items = 6;
#else
	m[4].type=NM_TYPE_RADIO; m[4].value=0; m[4].group=0; m[4].text=" 320x400";
	m[5].type=NM_TYPE_RADIO; m[5].value=0; m[5].group=0; m[5].text=" 640x480";
	m[6].type=NM_TYPE_RADIO; m[6].value=0; m[6].group=0; m[6].text=" 800x600";
	n_items = 7;
	if (FindArg("-superhires")) {
		m[7].type=NM_TYPE_RADIO; m[7].value=0; m[7].group=0; m[7].text=" 1024x768";
		m[8].type=NM_TYPE_RADIO; m[8].value=0; m[8].group=0; m[8].text=" 1280x1024";
		n_items += 2;
	}
#endif

	citem = Current_display_mode+1;
	
#ifdef WINDOWS
	if (citem == 3) citem++;				// if 320x400 in DOS, make it look like 640x400
#else
	if (Current_display_mode >= 2)
		citem++;
#endif

	if (citem >= n_items)
		citem = n_items-1;

	m[citem].value = 1;

	newmenu_do1( NULL, "Select screen mode", n_items, m, NULL, citem);

	for (i=0;i<n_items;i++)
		if (m[i].value)
			break;

#ifndef WINDOWS 								// if i >= 4 keep it that way since we skip 320x400
	if (i >= 4)
		i--;
#endif

	i--;

	#ifdef SHAREWARE
		if (i != 0)
			nm_messagebox(TXT_SORRY, 1, TXT_OK, 
				"High resolution modes are\n"
				"only available in the\n"
				"Commercial version of Descent 2.");
		return;
	#else
		if (i != Current_display_mode)
			set_display_mode(i);
	#endif

}
#endif	// end of PC version of do_screen_res_menu()



// Mission chooser for New Game. Replaces the DOS-era newmenu_listbox1(), which draws into a
// 320x200 offscreen buffer with no touch/OpenGL support. An ordinary newmenu with a few
// missions per page plus "Next/Previous Page" rows, so touch and gamepad both work.
// Returns the chosen index into Mission_list, or -1 if cancelled.
#define MISSIONS_PER_PAGE 6
static int do_mission_select_menu(int n_missions, int default_mission)
{
	newmenu_item m[MISSIONS_PER_PAGE + 2];
	int n_pages = (n_missions + MISSIONS_PER_PAGE - 1) / MISSIONS_PER_PAGE;
	int page = default_mission / MISSIONS_PER_PAGE;
	int citem = default_mission % MISSIONS_PER_PAGE;

	for (;;) {
		int first = page * MISSIONS_PER_PAGE;
		int count = n_missions - first;
		int n = 0, prev_row = -1, next_row = -1, choice, i;

		if (count > MISSIONS_PER_PAGE)
			count = MISSIONS_PER_PAGE;

		for (i = 0; i < count; i++) {
			m[n].type = NM_TYPE_MENU;
			m[n].text = Mission_list[first + i].mission_name;
			n++;
		}
		if (page > 0) {
			prev_row = n;
			m[n].type = NM_TYPE_MENU; m[n].text = "<< Previous Page";
			n++;
		}
		if (page < n_pages - 1) {
			next_row = n;
			m[n].type = NM_TYPE_MENU; m[n].text = "Next Page >>";
			n++;
		}

		choice = newmenu_do1("New Game", "Select mission", n, m, NULL, citem);

		if (choice < 0)
			return -1;
		if (choice == prev_row) {
			page--;
			citem = 0;
			continue;
		}
		if (choice == next_row) {
			page++;
			citem = 0;
			continue;
		}
		return first + choice;
	}
}

void do_new_game_menu()
{
	int new_level_num,player_highest_level;

#ifndef SHAREWARE
	int n_missions;

	n_missions = build_mission_list(0);

	if (n_missions > 1) {
		int new_mission_num,i, default_mission;
		char * m[MAX_MISSIONS];

		default_mission = 0;
		for (i=0;i<n_missions;i++) {
			m[i] = Mission_list[i].mission_name;
			if ( !strcasecmp( m[i], config_last_mission ) )
				default_mission = i;
		}

		new_mission_num = do_mission_select_menu( n_missions, default_mission );

		if (new_mission_num == -1)
			return;         //abort!

		strcpy(config_last_mission, m[new_mission_num]  );
		
		if (!load_mission(new_mission_num)) {
			nm_messagebox( NULL, 1, TXT_OK, "Error in Mission file\n\n%s", Mission_load_error);
			return;
		}
	}
#endif

	new_level_num = 1;

	player_highest_level = get_highest_level();

	if (player_highest_level > Last_level)
		player_highest_level = Last_level;

	if (player_highest_level > 1) {
		newmenu_item m[4];
		char info_text[80];
		char num_text[10];
		int choice;
		int n_items;

try_again:
		sprintf(info_text,"%s %d",TXT_START_ANY_LEVEL, player_highest_level);

		m[0].type=NM_TYPE_TEXT; m[0].text = info_text;
		m[1].type=NM_TYPE_INPUT; m[1].text_len = 10; m[1].text = num_text;
		n_items = 2;

		#ifdef WINDOWS
		m[2].type = NM_TYPE_TEXT; m[2].text = "";
		m[3].type = NM_TYPE_MENU; m[3].text = "          Ok";
		n_items = 4;
		#endif

		strcpy(num_text,"1");

		choice = newmenu_do( NULL, TXT_SELECT_START_LEV, n_items, m, NULL );

		if (choice==-1 || m[1].text[0]==0)
			return;

		new_level_num = atoi(m[1].text);

		if (!(new_level_num>0 && new_level_num<=player_highest_level)) {
			m[0].text = TXT_ENTER_TO_CONT;
			nm_messagebox( NULL, 1, TXT_OK, TXT_INVALID_LEVEL); 
			goto try_again;
		}
	}

	Difficulty_level = Player_default_difficulty;

	if (!do_difficulty_menu())
		return;

	gr_palette_fade_out( gr_palette, 32, 0 );
	StartNewGame(new_level_num);

}

extern void GameLoop(int, int );

			extern int Automap_always_hires;
			
#define ADD_CHECK(n,txt,v)  do { m[n].type=NM_TYPE_CHECK; m[n].text=txt; m[n].value=v;} while (0)
			
			void do_toggles_menu()
		{
#ifndef MACINTOSH
#if defined(POLY_ACC)
#define N_TOGGLE_ITEMS 6        // get rid of automap hi-res.
#else
#define N_TOGGLE_ITEMS 7
#endif
#else
#define N_TOGGLE_ITEMS 7
#endif
			newmenu_item m[N_TOGGLE_ITEMS];
			int i = 0;
			
			do {
#if defined(MACINTOSH) && defined(USE_ISP)
				if (ISpEnabled())
				{
					m[0].type = NM_TYPE_TEXT; m[0].text = "";
				}
				else
				{
					ADD_CHECK(0, "Ship auto-leveling", Auto_leveling_on);
				}
#else
				ADD_CHECK(0, "Ship auto-leveling", Auto_leveling_on);
#endif
				ADD_CHECK(1, "Show reticle", Reticle_on);
				ADD_CHECK(2, "Missile view", Missile_view_enabled);
				ADD_CHECK(3, "Headlight on when picked up", Headlight_active_default );
				ADD_CHECK(4, "Show guided missile in main display", Guided_in_big_window );
				ADD_CHECK(5, "Escort robot hot keys",EscortHotKeys);
#ifdef MACINTOSH
				if ( !PAEnabled ) {
					ADD_CHECK(6, "Pixel Double", Scanline_double);
				}
#else
#if !defined(POLY_ACC)
				ADD_CHECK(6, "Always show HighRes Automap", fmin(MenuHiresAvailable,Automap_always_hires));
#endif
#endif
				//when adding more options, change N_TOGGLE_ITEMS above
				
#ifdef MACINTOSH
				if ( PAEnabled )		// when doing RAVE, no pixel doubling
					i = newmenu_do1( NULL, "Toggles", N_TOGGLE_ITEMS-1, m, NULL, i );
				else
#endif		// note link to if
					i = newmenu_do1( NULL, "Toggles", N_TOGGLE_ITEMS, m, NULL, i );
				
				Auto_leveling_on			= m[0].value;
				Reticle_on					= m[1].value;
				Missile_view_enabled    	= m[2].value;
				Headlight_active_default	= m[3].value;
				Guided_in_big_window		= m[4].value;
				EscortHotKeys				= m[5].value;
				
				
#ifdef MACINTOSH
				if ( !PAEnabled )
					Scanline_double = m[6].value;
#else
#if !defined(POLY_ACC)
				if (MenuHiresAvailable)
					Automap_always_hires = m[6].value;
				else if (m[6].value)
					nm_messagebox(TXT_SORRY,1,"OK","High Resolution modes are\nnot available on this video card");
#endif
#endif
				
			} while( i>-1 );
			
		}
			
// Pause-menu Cheats submenu (do_cheats_menu() below), called from main/game.c's
// do_game_menu(). Every effect is the same code the typed cheat codes trigger, factored
// into standalone cheat_*() functions in main/gamecntl.c, so this menu is just a second
// front end onto the same implementation (including the cheat score penalty).
extern void cheat_toggle_invulnerability(void);
extern void cheat_toggle_cloak(void);
extern void cheat_fill_shields(void);
extern void cheat_grant_all_keys(void);
extern void cheat_full_arsenal(void);
extern void cheat_extra_life(void);
extern void cheat_toggle_ghost_mode(void);
extern void cheat_toggle_turbo_mode(void);
extern void cheat_toggle_robot_firing(void);
extern void cheat_warp_to_level(int new_level_num);
extern int Physics_cheat_flag;
extern void cheat_grant_accessories(void);
extern void cheat_full_map(void);
extern void cheat_toggle_homing(void);
extern void cheat_toggle_bouncy(void);
extern void cheat_toggle_rapid_fire(void);
extern void cheat_toggle_rabid_robots(void);
extern void cheat_toggle_monster_mode(void);
extern void cheat_destroy_all_robots(void);
extern void cheat_summon_guidebot(void);
extern char HomingCheat, BounceCheat, Monster_mode;
extern void cheat_toggle_angry_guidebot(void);
extern void cheat_toggle_acid(void);
extern char AcidCheatOn;
extern int Buddy_dude_cheat;
extern int Laser_rapid_fire, Robots_kill_robots_cheat;

// Second page of the pause-menu cheats: the rest of Descent 2's typed cheat codes.
static void do_more_cheats_menu(void)
{
	newmenu_item m[12];
	int was_homing, was_bouncy, was_rapid, was_rabid, was_monster, was_wingnut, was_acid;
	int i = 0;

	do {
		was_homing = (HomingCheat != 0);
		was_bouncy = (BounceCheat != 0);
		was_rapid = (Laser_rapid_fire != 0);
		was_rabid = (Robots_kill_robots_cheat != 0);
		was_monster = (Monster_mode != 0);
		was_wingnut = (Buddy_dude_cheat != 0);
		was_acid = (AcidCheatOn != 0);

		m[0].type = NM_TYPE_CHECK; m[0].text = "Homing Weapons"; m[0].value = was_homing;
		m[1].type = NM_TYPE_CHECK; m[1].text = "Bouncing Weapons"; m[1].value = was_bouncy;
		m[2].type = NM_TYPE_CHECK; m[2].text = "Rapid Fire"; m[2].value = was_rapid;
		m[3].type = NM_TYPE_CHECK; m[3].text = "Robots Fight Each Other"; m[3].value = was_rabid;
		m[4].type = NM_TYPE_CHECK; m[4].text = "Godzilla"; m[4].value = was_monster;
		m[5].type = NM_TYPE_CHECK; m[5].text = "Angry Guide-Bot"; m[5].value = was_wingnut;
		m[6].type = NM_TYPE_CHECK; m[6].text = "Acid Mode"; m[6].value = was_acid;
		m[7].type = NM_TYPE_TEXT; m[7].text = "";
		m[8].type = NM_TYPE_MENU; m[8].text = "Accessories";
		m[9].type = NM_TYPE_MENU; m[9].text = "Full Map";
		m[10].type = NM_TYPE_MENU; m[10].text = "Destroy All Robots";
		m[11].type = NM_TYPE_MENU; m[11].text = "Summon Guide-Bot";

		i = newmenu_do1(NULL, "More Cheats", 12, m, NULL, i);

		if (m[0].value != was_homing) cheat_toggle_homing();
		if (m[1].value != was_bouncy) cheat_toggle_bouncy();
		if (m[2].value != was_rapid) cheat_toggle_rapid_fire();
		if (m[3].value != was_rabid) cheat_toggle_rabid_robots();
		if (m[4].value != was_monster) cheat_toggle_monster_mode();
		if (m[5].value != was_wingnut) cheat_toggle_angry_guidebot();
		if (m[6].value != was_acid) cheat_toggle_acid();

		if (i == 8) cheat_grant_accessories();
		if (i == 9) cheat_full_map();
		if (i == 10) cheat_destroy_all_robots();
		if (i == 11) cheat_summon_guidebot();
	} while (i > -1);
}
extern int Game_turbo_mode;
extern int Robot_firing_enabled;

void do_cheats_menu(void)
{
	newmenu_item m[12];
	// Captured fresh on every redraw so a flag that changes on its own (invulnerability
	// timing out) shows correctly, and compared after newmenu_do1() returns so a toggle
	// only fires for a checkbox the player actually touched.
	int was_invuln, was_cloaked, was_ghost, was_turbo, was_robots_fire;
	int i = 0;

	do {
		was_invuln = (Players[Player_num].flags & PLAYER_FLAGS_INVULNERABLE) != 0;
		was_cloaked = (Players[Player_num].flags & PLAYER_FLAGS_CLOAKED) != 0;
		was_ghost = (Physics_cheat_flag == 0xBADA55);
		was_turbo = (Game_turbo_mode != 0);
		was_robots_fire = (Robot_firing_enabled != 0);

		m[0].type = NM_TYPE_CHECK; m[0].text = "Invulnerability"; m[0].value = was_invuln;
		m[1].type = NM_TYPE_CHECK; m[1].text = "Cloak"; m[1].value = was_cloaked;
		m[2].type = NM_TYPE_CHECK; m[2].text = "Ghost Mode (no clip)"; m[2].value = was_ghost;
		m[3].type = NM_TYPE_CHECK; m[3].text = "Turbo Mode"; m[3].value = was_turbo;
		m[4].type = NM_TYPE_CHECK; m[4].text = "Robots Can Fire"; m[4].value = was_robots_fire;
		m[5].type = NM_TYPE_TEXT; m[5].text = "";
		m[6].type = NM_TYPE_MENU; m[6].text = "Full Shields";
		m[7].type = NM_TYPE_MENU; m[7].text = "All Keys";
		m[8].type = NM_TYPE_MENU; m[8].text = "Full Arsenal";
		m[9].type = NM_TYPE_MENU; m[9].text = "Extra Life";
		m[10].type = NM_TYPE_MENU; m[10].text = "Warp to Level...";
		m[11].type = NM_TYPE_MENU; m[11].text = "More Cheats...";

		i = newmenu_do1(NULL, "Cheats", 12, m, NULL, i);

		if (m[0].value != was_invuln) cheat_toggle_invulnerability();
		if (m[1].value != was_cloaked) cheat_toggle_cloak();
		if (m[2].value != was_ghost) cheat_toggle_ghost_mode();
		if (m[3].value != was_turbo) cheat_toggle_turbo_mode();
		if (m[4].value != was_robots_fire) cheat_toggle_robot_firing();

		if (i == 6) cheat_fill_shields();
		if (i == 7) cheat_grant_all_keys();
		if (i == 8) cheat_full_arsenal();
		if (i == 9) cheat_extra_life();
		if (i == 11) do_more_cheats_menu();
		if (i == 10) {
			// Same "type a level number" prompt the typed warp cheat uses; the
			// cheat_warp_to_level() function owns validation and the jump itself.
			newmenu_item wm;
			char text[10] = "";
			int item;
			wm.type = NM_TYPE_INPUT; wm.text_len = 10; wm.text = text;
			item = newmenu_do(NULL, TXT_WARP_TO_LEVEL, 1, &wm, NULL);
			if (item != -1) {
				cheat_warp_to_level(atoi(text));
				// StartNewLevel() rebuilds all level/render/briefing state; looping back to
				// redraw this menu over that in-flight rebuild caused a missing briefing
				// background, scrambled colors and the app minimizing. Bail out instead.
				return;
			}
		}
	} while (i > -1);
}

// Set by do_options_menu() before each menu build so joydef_menuset() (called while the menu
// is live) knows where the Brightness slider currently is -- its index shifts depending on
// whether the "Use Gyroscope" entry is present.
static int Options_menu_have_gyroscope = 0;
// Row of the Touch Scaling / Touch Opacity sliders and the Brightness slider in the Options menu
// currently on screen (-1 when absent: the touch sliders are only offered inside a level, where
// the buttons can actually be seen).
static int Options_touch_scale_item = -1;
static int Options_touch_opacity_item = -1;
static int Options_brightness_item = -1;
#ifdef ANDROID_NDK
int Touch_preview_wanted = 0;
#endif

void joydef_menuset(int nitems, newmenu_item * items, int *last_key, int citem )
{
	int brightness_item = Options_brightness_item;

	nitems=nitems;
	*last_key = *last_key;

	if ( citem == brightness_item )	{
		gr_palette_set_gamma(items[brightness_item].value);
	}

#ifdef ANDROID_NDK
	// Touch Scaling / Opacity take effect live as the sliders move, and while one is the selected
	// item the touch buttons are previewed behind the menu (see newmenu_do4_inner()).
	if (Options_touch_scale_item >= 0) {
		if (Config_touch_control_scale != items[Options_touch_scale_item].value) {
			Config_touch_control_scale = (ubyte) items[Options_touch_scale_item].value;
			touch_control_scale_changed();
		}
		Config_touch_control_opacity = (ubyte) items[Options_touch_opacity_item].value;
		Touch_preview_wanted = (citem == Options_touch_scale_item || citem == Options_touch_opacity_item);
	} else {
		Touch_preview_wanted = 0;
	}
#endif

	if ( Config_digi_volume != items[0].value )	{
		Config_digi_volume = items[0].value;
		digi_set_digi_volume( (Config_digi_volume*32768)/8 );
		digi_play_sample_once( SOUND_DROP_BOMB, F1_0 );
	}

	if (Config_midi_volume != items[1].value )	{
		Config_midi_volume = items[1].value;
		digi_set_midi_volume( (Config_midi_volume*128)/8 );
	}
}

void do_options_menu()
{
	newmenu_item m[16];
	int i = 0;
	int open_mission_pack_picker = 0;
	int have_gyroscope = haveGyroscope();
	// The touch sliders only mean something where the on-screen buttons are visible, i.e. in a level.
	int have_touch = (Function_mode == FMODE_GAME);
	int n, remap_item, sens_item, invert_item, gyro_item, video_item, toggles_item, packs_item;
	Options_menu_have_gyroscope = have_gyroscope;

	do {
		n = 0;
		m[n].type = NM_TYPE_SLIDER; m[n].text=TXT_FX_VOLUME; m[n].value=Config_digi_volume;m[n].min_value=0; m[n].max_value=8; n++;
		m[n].type = NM_TYPE_SLIDER; m[n].text=TXT_MUSIC_VOLUME; m[n].value=Config_midi_volume;m[n].min_value=0; m[n].max_value=8; n++;
		m[n].type = NM_TYPE_TEXT; m[n].text=""; n++;
		remap_item = n;
		m[n].type = NM_TYPE_MENU; m[n].text="Remap Gamepad"; n++;
		sens_item = n;
		m[n].type = NM_TYPE_SLIDER; m[n].text="Look Sensitivity"; m[n].value=Config_joystick_sensitivity; m[n].min_value =0; m[n].max_value = 8; n++;
		if (have_touch) {
			Options_touch_scale_item = n;
			m[n].type = NM_TYPE_SLIDER; m[n].text="Touch Scaling"; m[n].value=Config_touch_control_scale; m[n].min_value=0; m[n].max_value=8; n++;
			Options_touch_opacity_item = n;
			m[n].type = NM_TYPE_SLIDER; m[n].text="Touch Opacity"; m[n].value=Config_touch_control_opacity; m[n].min_value=0; m[n].max_value=7; n++;
		} else {
			Options_touch_scale_item = -1;
			Options_touch_opacity_item = -1;
		}
		invert_item = n;
		m[n].type = NM_TYPE_CHECK; m[n].text="Invert Y"; m[n].value=Config_invert_y; n++;
		m[n].type = NM_TYPE_TEXT; m[n].text=""; n++;
		// Ship auto-leveling lives only in the Toggles menu.
		gyro_item = -1;
		if (have_gyroscope) {
			gyro_item = n;
			m[n].type = NM_TYPE_CHECK; m[n].text = "Use Gyroscope"; m[n].value = Config_use_gyroscope; n++;
		}
		m[n].type = NM_TYPE_TEXT; m[n].text=""; n++;
		Options_brightness_item = n;
		m[n].type = NM_TYPE_SLIDER; m[n].text=TXT_BRIGHTNESS; m[n].value=gr_palette_get_gamma();m[n].min_value=0; m[n].max_value=8; n++;
		video_item = n;
		m[n].type = NM_TYPE_MENU; m[n].text="Video Options"; n++;
		toggles_item = n;
		m[n].type = NM_TYPE_MENU; m[n].text="Toggles..."; n++;
		packs_item = n;
		m[n].type = NM_TYPE_MENU; m[n].text="Add Mission Packs"; n++;

		i = newmenu_do1( NULL, TXT_OPTIONS, n, m, joydef_menuset, i );
#ifdef ANDROID_NDK
		Touch_preview_wanted = 0;
#endif

		if (i == remap_item) {
			do_remap_gamepad_menu();
		}

		if (i == video_item) {
			do_detail_level_menu_custom();
		}

		if (i == toggles_item) {
			do_toggles_menu();
		}

		if (i == packs_item) {
			// Android's folder picker pauses the app, which destroys the screen contents
			// saved under this menu. Close Options first and launch the picker once out.
			open_mission_pack_picker = 1;
			i = -1;
		}

		Config_joystick_sensitivity = m[sens_item].value;
		if (have_touch) {
			if (Config_touch_control_scale != m[Options_touch_scale_item].value) {
				Config_touch_control_scale = (ubyte) m[Options_touch_scale_item].value;
				touch_control_scale_changed();
			}
			Config_touch_control_opacity = (ubyte) m[Options_touch_opacity_item].value;
		}
		Config_invert_y = m[invert_item].value;
		if (have_gyroscope) {
			if (Config_use_gyroscope != m[gyro_item].value) {
				if (m[gyro_item].value) {
					startMotion();
				} else {
					stopMotion();
				}
			}
			Config_use_gyroscope = m[gyro_item].value;
		}
	} while( i>-1 );

	if ( Config_midi_volume < 1 )	{
		digi_play_midi_song( NULL, NULL, NULL, 0 );
	}

	write_player_file();

	if (open_mission_pack_picker)
		openMissionPackPicker();
}

void do_multi_player_menu()
{
	int menu_choice[5];
	newmenu_item m[5];
	int choice = 0, num_options = 0;
	int old_game_mode;

	do {
//		WIN(ipx_destroy_read_thread());

		old_game_mode = Game_mode;
		num_options = 0;

		ADD_ITEM(TXT_START_IPX_NET_GAME, MENU_START_IPX_NETGAME, -1 );
		ADD_ITEM(TXT_JOIN_IPX_NET_GAME, MENU_JOIN_IPX_NETGAME, -1 );
              //  ADD_ITEM(TXT_START_TCP_NET_GAME, MENU_START_TCP_NETGAME, -1 );
              //  ADD_ITEM(TXT_JOIN_TCP_NET_GAME, MENU_JOIN_TCP_NETGAME, -1 );

        #ifdef MACINTOSH
		ADD_ITEM("Start Appletalk Netgame", MENU_START_APPLETALK_NETGAME, -1 );
		ADD_ITEM("Join Appletalk Netgame\n", MENU_JOIN_APPLETALK_NETGAME, -1 );
        #endif

		ADD_ITEM(TXT_MODEM_GAME, MENU_START_SERIAL, -1);

		choice = newmenu_do1( NULL, TXT_MULTIPLAYER, num_options, m, NULL, choice );
		
		if ( choice > -1 )      
			do_option(menu_choice[choice]);
	
		if (old_game_mode != Game_mode)
			break;          // leave menu

	} while( choice > -1 );

}

void DoNewIPAddress ()
 {
  newmenu_item m[4];
  char IPText[30];
  int choice;

  m[0].type=NM_TYPE_TEXT; m[0].text = "Enter an address or hostname:";
  m[1].type=NM_TYPE_INPUT; m[1].text_len = 50; m[1].text = IPText;
  IPText[0]=0;

  choice = newmenu_do( NULL, "Join a TCPIP game", 2, m, NULL );

  if (choice==-1 || m[1].text[0]==0)
   return;

  nm_messagebox (TXT_SORRY,1,TXT_OK,"That address is not valid!");
 }


  

