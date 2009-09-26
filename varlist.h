#ifdef OWLVAR_ONLY_NAMES

#define OWLVAR_BOOL_FULL(name,cname,a,b,c,d,e,f)   OWLVAR_BOOL(name,cname,a,b,c)

#define OWLVAR_PATH                                OWLVAR_STRING
#define OWLVAR_STRING_FULL(name,cname,a,b,c,d,e,f) OWLVAR_STRING(name,cname,a,b,c)

#define OWLVAR_INT_FULL(name,cname,a,b,c,d,e,f,g)  OWLVAR_INT(name,cname,a,b,c)
#define OWLVAR_ENUM(name,cname,a,b,c,d)            OWLVAR_INT(name,cname,a,b,c)
#define OWLVAR_ENUM_FULL(name,cname,a,b,c,d,e,f,g) OWLVAR_ENUM(name,cname,a,b,c,d)

#endif

OWLVAR_STRING( "personalbell", personalbell, "off",
               "ring the terminal bell when personal messages are received",
               "Can be set to 'on', 'off', or the name of a filter which\n"
               "messages need to match in order to ring the bell")

OWLVAR_BOOL( "bell", bell, 1,
             "enable / disable the terminal bell", "" )

OWLVAR_BOOL_FULL( "debug", debug, OWL_DEBUG,
                  "whether debugging is enabled",
                  "If set to 'on', debugging messages are logged to the\n"
                  "file specified by the debugfile variable.\n",
                  NULL, owl_variable_debug_set, NULL)

OWLVAR_BOOL( "startuplogin", startuplogin, 1,
             "send a login message when owl starts", "" )

OWLVAR_BOOL( "shutdownlogout", shutdownlogout, 1,
             "send a logout message when owl exits", "" )

OWLVAR_BOOL( "rxping", rxping, 0,
             "display received pings", "" )

OWLVAR_BOOL( "txping", txping, 1,
             "send pings", "" )

OWLVAR_BOOL( "sepbar_disable", sepbar_disable, 0,
             "disable printing information in the separator bar", "" )

OWLVAR_BOOL( "smartstrip", smartstrip, 1,
             "strip kerberos instance for reply", "")

OWLVAR_BOOL( "newlinestrip", newlinestrip, 1,
             "strip leading and trailing newlines", "")

OWLVAR_BOOL( "displayoutgoing", displayoutgoing, 1,
             "display outgoing messages", "" )

OWLVAR_BOOL( "loginsubs", loginsubs, 1,
             "load logins from .anyone on startup", "" )

OWLVAR_BOOL( "logging", logging, 0,
             "turn personal logging on or off",
             "If this is set to on, personal messages are\n"
             "logged in the directory specified\n"
             "by the 'logpath' variable.  The filename in that\n"
             "directory is derived from the sender of the message.\n" )

OWLVAR_BOOL( "classlogging", classlogging, 0,
             "turn class logging on or off",
             "If this is set to on, class messages are\n"
             "logged in the directory specified\n"
             "by the 'classlogpath' variable.\n"
             "The filename in that directory is derived from\n"
             "the name of the class to which the message was sent.\n" )

OWLVAR_ENUM( "loggingdirection", loggingdirection, OWL_LOGGING_DIRECTION_BOTH,
             "specifices which kind of messages should be logged",
             "Can be one of 'both', 'in', or 'out'.  If 'in' is\n"
             "selected, only incoming messages are logged, if 'out'\n"
             "is selected only outgoing messages are logged.  If 'both'\n"
             "is selected both incoming and outgoing messages are\n"
             "logged.",
             "both,in,out")

OWLVAR_BOOL( "colorztext", colorztext, 1,
             "allow @color() in zephyrs to change color",
             "Note that only messages received after this variable\n"
             "is set will be affected." )

OWLVAR_BOOL( "fancylines", fancylines, 1,
             "Use 'nice' line drawing on the terminal.",
             "If turned off, dashes, pipes and pluses will be used\n"
             "to draw lines on the screen.  Useful when the terminal\n"
             "is causing problems" )

OWLVAR_BOOL( "zcrypt", zcrypt, 1,
             "Do automatic zcrypt processing",
             "" )

OWLVAR_BOOL_FULL( "pseudologins", pseudologins, 0,
                  "Enable zephyr pseudo logins",
                  "When this is enabled, Owl will periodically check the zephyr\n"
                  "location of users in your .anyone file.  If a user is present\n"
                  "but sent no login message, or a user is not present that sent no\n"
                  "logout message, a pseudo login or logout message wil be created\n",
                  NULL, owl_variable_pseudologins_set, NULL)

OWLVAR_BOOL( "ignorelogins", ignorelogins, 0,
             "Enable printing of login notifications",
             "When this is enabled, Owl will print login and logout notifications\n"
             "for AIM, zephyr, or other protocols.  If disabled Owl will not print\n"
             "login or logout notifications.\n")

OWLVAR_STRING( "logfilter", logfilter, "",
               "name of a filter controlling which messages to log",

               "If non empty, any messages matching the given filter will be logged.\n"
               "This is a completely separate mechanisim from the other logging\n"
               "variables like logging, classlogging, loglogins, loggingdirection,\n"
               "etc.  If you want this variable to control all logging, make sure\n"
               "all other logging variables are in their default state.\n")

OWLVAR_BOOL( "loglogins", loglogins, 0,
             "Enable logging of login notifications",
             "When this is enabled, Owl will login login and logout notifications\n"
             "for AIM, zephyr, or other protocols.  If disabled Owl will not print\n"
             "login or logout notifications.\n")

OWLVAR_ENUM_FULL( "disable-ctrl-d", lockout_ctrld, 1,
                  "don't send zephyrs on C-d",
                  "If set to 'off', C-d won't send a zephyr from the edit\n"
                  "window.  If set to 'on', C-d will always send a zephyr\n"
                  "being composed in the edit window.  If set to 'middle',\n"
                  "C-d will only ever send a zephyr if the cursor is at\n"
                  "the end of the message being composed.\n\n"
                  "Note that this works by changing the C-d keybinding\n"
                  "in the editmulti keymap.\n",
                  "off,middle,on",
                  NULL, owl_variable_disable_ctrl_d_set, NULL)

OWLVAR_PATH( "logpath", logpath, "~/zlog/people",
             "path for logging personal zephyrs",
             "Specifies a directory which must exist.\n"
             "Files will be created in the directory for each sender.\n")

OWLVAR_PATH( "classlogpath", classlogpath, "~/zlog/class",
             "path for logging class zephyrs",
             "Specifies a directory which must exist.\n"
             "Files will be created in the directory for each class.\n")

OWLVAR_PATH( "debug_file", debug_file, OWL_DEBUG_FILE,
             "path for logging debug messages when debugging is enabled",
             "This file will be logged to if 'debug' is set to 'on'.\n")

OWLVAR_PATH( "zsigproc", zsigproc, NULL,
             "name of a program to run that will generate zsigs",
             "This program should produce a zsig on stdout when run.\n"
             "Note that it is important that this program not block.\n\n"
             "See the documentation for 'zsig' for more information about\n"
             "how the outgoing zsig is chosen."
             )

OWLVAR_PATH( "newmsgproc", newmsgproc, NULL,
             "name of a program to run when new messages are present",
             "The named program will be run when owl recevies new.\n"
             "messages.  It will not be run again until the first\n"
             "instance exits")

OWLVAR_STRING( "zsender", zsender, "",
               "zephyr sender name",
               "Allows you to customize the outgoing username in\n"
               "zephyrs.  If this is unset, it will use your Kerberos\n"
               "principal. Note that customizing the sender name will\n"
               "cause your zephyrs to be sent unauthenticated.")

OWLVAR_STRING( "zsig", zsig, "",
               "zephyr signature",
               "The zsig to get on outgoing messages. If this variable is\n"
               "unset, 'zsigproc' will be run to generate a zsig. If that is\n"
               "also unset, the 'zwrite-signature' zephyr variable will be\n"
               "used instead.\n")

OWLVAR_STRING( "appendtosepbar", appendtosepbar, "",
               "string to append to the end of the sepbar",
               "The sepbar is the bar separating the top and bottom\n"
               "of the owl screen.  Any string specified here will\n"
               "be displayed on the right of the sepbar\n")

OWLVAR_BOOL( "zaway", zaway, 0,
             "turn zaway on or off", "" )

OWLVAR_STRING( "zaway_msg", zaway_msg,
               OWL_DEFAULT_ZAWAYMSG,
               "zaway msg for responding to zephyrs when away", "" )

OWLVAR_STRING( "zaway_msg_default", zaway_msg_default,
               OWL_DEFAULT_ZAWAYMSG,
               "default zaway message", "" )

OWLVAR_BOOL_FULL( "aaway", aaway, 0,
                  "Set AIM away status",
                  "",
                  NULL, owl_variable_aaway_set, NULL)

OWLVAR_STRING( "aaway_msg", aaway_msg,
               OWL_DEFAULT_AAWAYMSG,
               "AIM away msg for responding when away", "" )

OWLVAR_STRING( "aaway_msg_default", aaway_msg_default,
               OWL_DEFAULT_AAWAYMSG,
               "default AIM away message", "" )

OWLVAR_STRING( "view_home", view_home, "all",
               "home view to switch to after 'X' and 'V'",
               "SEE ALSO: view, filter\n" )

OWLVAR_STRING( "alert_filter", alert_filter, "none",
               "filter on which to trigger alert actions",
               "" )

OWLVAR_STRING( "alert_action", alert_action, "nop",
               "owl command to execute for alert actions",
               "" )

OWLVAR_STRING_FULL( "tty", tty, "", "tty name for zephyr location", "",
                    NULL, owl_variable_tty_set, NULL)

OWLVAR_STRING( "default_style", default_style, "__unspecified__",
               "name of the default formatting style",
               "This sets the default message formatting style.\n"
               "Styles may be created with the 'style' command.\n"
               "Some built-in styles include:\n"
               "   default  - the default owl formatting\n"
               "   oneline  - one line per-message\n"
               "   perl     - legacy perl interface\n"
               "\nSEE ALSO: style, show styles, view -s <style>\n"
               )


OWLVAR_INT(    "edit:maxfillcols", edit_maxfillcols, 70,
               "maximum number of columns for M-q to fill text to",
               "This specifies the maximum number of columns for M-q\n"
               "to fill text to.  If set to 0, ther will be no maximum\n"
               "limit.  In all cases, the current width of the screen\n"
               "will also be taken into account.  It will be used instead\n"
               "if it is narrower than the maximum, or if this\n"
               "is set to 0.\n" )

OWLVAR_INT(    "edit:maxwrapcols", edit_maxwrapcols, 0,
               "maximum number of columns for line-wrapping",
               "This specifies the maximum number of columns for\n"
               "auto-line-wrapping.  If set to 0, ther will be no maximum\n"
               "limit.  In all cases, the current width of the screen\n"
               "will also be taken into account.  It will be used instead\n"
               "if it is narrower than the maximum, or if this\n"
               "is set to 0.\n\n"
               "It is recommended that outgoing messages be no wider\n"
               "than 60 columns, as a courtesy to recipients.\n")

OWLVAR_INT( "aim_ignorelogin_timer", aim_ignorelogin_timer, 15,
            "number of seconds after AIM login to ignore login messages",
            "This specifies the number of seconds to wait after an\n"
            "AIM login before allowing the recipt of AIM login notifications.\n"
            "By default this is set to 15.  If you would like to view login\n"
            "notifications of buddies as soon as you login, set it to 0 instead.")


OWLVAR_INT_FULL( "typewinsize", typwin_lines,
                 OWL_TYPWIN_SIZE,
                 "number of lines in the typing window",
                 "This specifies the height of the window at the\n"
                 "bottom of the screen where commands are entered\n"
                 "and where messages are composed.\n",
                 "int > 0",
                 owl_variable_int_validate_gt0,
                 owl_variable_typewinsize_set,
                 NULL /* use default for get */
                 )

OWLVAR_INT( "typewindelta", typewindelta, 0,
            "number of lines to add to the typing window when in use",
            "On small screens you may want the typing window to\n"
            "auto-hide when not entering a command or message.\n"
            "This variable is the number of lines to add to the\n"
            "typing window when it is in use; you can then set\n"
            "typewinsize to 1.\n\n"
            "This works a lot better with a non-default scrollmode;\n"
            "try :set scrollmode pagedcenter.\n")

OWLVAR_ENUM( "scrollmode", scrollmode, OWL_SCROLLMODE_NORMAL,
             "how to scroll up and down",
             "This controls how the screen is scrolled as the\n"
             "cursor moves between messages being displayed.\n"
             "The following modes are supported:\n\n"
             "   normal      - This is the owl default.  Scrolling happens\n"
             "                 when it needs to, and an attempt is made to\n"
             "                 keep the current message roughly near\n"
             "                 the middle of the screen.\n"
             "   top         - The current message will always be the\n"
             "                 the top message displayed.\n"
             "   neartop     - The current message will be one down\n"
             "                 from the top message displayed,\n"
             "                 where possible.\n"
             "   center      - An attempt is made to keep the current\n"
             "                 message near the center of the screen.\n"
             "   paged       - The top message displayed only changes\n"
             "                 when user moves the cursor to the top\n"
             "                 or bottom of the screen.  When it moves,\n"
             "                 the screen will be paged up or down and\n"
             "                 the cursor will be near the top or\n"
             "                 the bottom.\n"
             "   pagedcenter - The top message displayed only changes\n"
             "                 when user moves the cursor to the top\n"
             "                 or bottom of the screen.  When it moves,\n"
             "                 the screen will be paged up or down and\n"
             "                 the cursor will be near the center.\n",
             "normal,top,neartop,center,paged,pagedcenter" )


OWLVAR_BOOL( "_followlast", _followlast, 0,
             "enable automatic following of the last zephyr",
             "If the cursor is at the last message, it will\n"
             "continue to follow the last message if this is set.\n"
             "Note that this is currently risky as you might accidentally\n"
             "delete a message right as it came in.\n" )
