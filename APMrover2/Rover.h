/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/* 
   main Rover class, containing all vehicle specific state
*/

class Rover {
public:
    Rover(void);
    // public member functions
    void setup(void);
    void loop(void);

private:
    AP_HAL::BetterStream* cliSerial;

    // must be the first AP_Param variable declared to ensure its
    // constructor runs before the constructors of the other AP_Param
    // variables
    AP_Param param_loader;

    // the rate we run the main loop at
    const AP_InertialSensor::Sample_rate ins_sample_rate = AP_InertialSensor::RATE_50HZ;

    // all settable parameters
    Parameters g;

    // main loop scheduler
    AP_Scheduler scheduler;

    // mapping between input channels
    RCMapper rcmap;

    // board specific config
    AP_BoardConfig BoardConfig;

    // primary control channels
    RC_Channel *channel_steer;
    RC_Channel *channel_throttle;
    RC_Channel *channel_learn;

    // DataFlash
#if CONFIG_HAL_BOARD == HAL_BOARD_APM1
    DataFlash_APM1 DataFlash;
#elif CONFIG_HAL_BOARD == HAL_BOARD_APM2
    DataFlash_APM2 DataFlash;
#elif defined(HAL_BOARD_LOG_DIRECTORY)
    DataFlash_File DataFlash(HAL_BOARD_LOG_DIRECTORY);
#else
    DataFlash_Empty DataFlash;
#endif

    bool in_log_download;

    // sensor drivers
    AP_GPS gps;
    AP_Baro barometer;
    Compass compass;
#if CONFIG_HAL_BOARD == HAL_BOARD_APM1
    AP_ADC_ADS7844 apm1_adc;
#endif
    AP_InertialSensor ins;
    RangeFinder sonar;

    // flight modes convenience array
    AP_Int8	*modes;

    // Inertial Navigation EKF
#if AP_AHRS_NAVEKF_AVAILABLE
    AP_AHRS_NavEKF ahrs;
#else
    AP_AHRS_DCM ahrs;
#endif

    AP_L1_Control L1_controller;

    // selected navigation controller
    AP_Navigation *nav_controller;

    // steering controller
    AP_SteerController steerController;

    // Mission library
    AP_Mission mission;

    OpticalFlow optflow;

#if CONFIG_HAL_BOARD == HAL_BOARD_SITL
    SITL sitl;
#endif

    // GCS handling
    AP_SerialManager serial_manager;
    const uint8_t num_gcs = MAVLINK_COMM_NUM_BUFFERS;
    GCS_MAVLINK gcs[MAVLINK_COMM_NUM_BUFFERS];

    // a pin for reading the receiver RSSI voltage. The scaling by 0.25 
    // is to take the 0 to 1024 range down to an 8 bit range for MAVLink
    AP_HAL::AnalogSource *rssi_analog_source;

    // relay support
    AP_Relay relay;

    AP_ServoRelayEvents ServoRelayEvents;

    // Camera
#if CAMERA == ENABLED
    AP_Camera camera;
#endif

    // The rover's current location
    struct Location current_loc;

    // Camera/Antenna mount tracking and stabilisation stuff
#if MOUNT == ENABLED
    // current_loc uses the baro/gps soloution for altitude rather than gps only.
    AP_Mount camera_mount;
#endif

    // if USB is connected
    bool usb_connected;

    // Radio
    // This is the state of the flight control system
    // There are multiple states defined such as MANUAL, FBW-A, AUTO
    enum mode control_mode;

    // Used to maintain the state of the previous control switch position
    // This is set to -1 when we need to re-read the switch
    uint8_t oldSwitchPosition;

    // These are values received from the GCS if the user is using GCS joystick
    // control and are substituted for the values coming from the RC radio
    int16_t rc_override[8];

    // A flag if GCS joystick control is in use
    bool rc_override_active;

    // Failsafe
    // A tracking variable for type of failsafe active
    // Used for failsafe based on loss of RC signal or GCS signal. See 
    // FAILSAFE_EVENT_*
    static struct {
        uint8_t bits;
        uint32_t rc_override_timer;
        uint32_t start_time;
        uint8_t triggered;
        uint32_t last_valid_rc_ms;
    } failsafe;

    // notification object for LEDs, buzzers etc (parameter set to false disables external leds)
    AP_Notify notify;

    // A counter used to count down valid gps fixes to allow the gps estimate to settle
    // before recording our home position (and executing a ground start if we booted with an air start)
    uint8_t ground_start_count;

    // Location & Navigation
    const float radius_of_earth = 6378100;	// meters

    // true if we have a position estimate from AHRS
    bool have_position;

    bool rtl_complete;

    // angle of our next navigation waypoint
    int32_t next_navigation_leg_cd;

    // ground speed error in m/s
    float groundspeed_error;	

    // 0-(throttle_max - throttle_cruise) : throttle nudge in Auto mode using top 1/2 of throttle stick travel
    int16_t     throttle_nudge;

    // receiver RSSI
    uint8_t receiver_rssi;

    // the time when the last HEARTBEAT message arrived from a GCS
    uint32_t last_heartbeat_ms;

    // obstacle detection information
    struct {
        // have we detected an obstacle?
        uint8_t detected_count;
        float turn_angle;
        uint16_t sonar1_distance_cm;
        uint16_t sonar2_distance_cm;
        
        // time when we last detected an obstacle, in milliseconds
        uint32_t detected_time_ms;
    } obstacle;

    // this is set to true when auto has been triggered to start
    bool auto_triggered;

    // Ground speed
    // The amount current ground speed is below min ground speed.  meters per second
    float 	ground_speed;
    int16_t throttle_last;
    int16_t throttle;

    // CH7 control
    // Used to track the CH7 toggle state.
    // When CH7 goes LOW PWM from HIGH PWM, this value will have been set true
    // This allows advanced functionality to know when to execute
    bool ch7_flag;

    // Battery Sensors
    AP_BattMonitor battery;

    // Battery Sensors
#if FRSKY_TELEM_ENABLED == ENABLED
    AP_Frsky_Telem frsky_telemetry;
#endif

    // Navigation control variables
    // The instantaneous desired lateral acceleration in m/s/s
    float lateral_acceleration;

    // Waypoint distances
    // Distance between rover and next waypoint.  Meters
    float wp_distance;
    // Distance between previous and next waypoint.  Meters
    int32_t wp_totalDistance;

    // Conditional command
    // A value used in condition commands (eg delay, change alt, etc.)
    // For example in a change altitude command, it is the altitude to change to.
    int32_t condition_value;
    // A starting value used to check the status of a conditional command.
    // For example in a delay command the condition_start records that start time for the delay
    int32_t	condition_start;

    // 3D Location vectors
    // Location structure defined in AP_Common
    // The home location used for RTL.  The location is set when we first get stable GPS lock
    const struct Location &home;

    // Flag for if we have gps lock and have set the home location
    bool home_is_set;
    // The location of the previous waypoint.  Used for track following and altitude ramp calculations
    struct Location prev_WP;
    // The location of the current/active waypoint.  Used for track following
    struct Location next_WP;
    // The location of the active waypoint in Guided mode.
    struct Location guided_WP;

    // IMU variables
    // The main loop execution time.  Seconds
    // This is the time between calls to the DCM algorithm and is the Integration time for the gyros.
    float G_Dt;		

    // Performance monitoring
    // Timer used to accrue data and trigger recording of the performanc monitoring log message
    int32_t	perf_mon_timer;
    // The maximum main loop execution time recorded in the current performance monitoring interval
    uint32_t G_Dt_max;

    // System Timers
    // Time in microseconds of start of main control loop. 
    uint32_t fast_loopTimer_us;
    // Number of milliseconds used in last main loop cycle
    uint32_t delta_us_fast_loop;
    // Counter of main loop executions.  Used for performance monitoring and failsafe processing
    uint16_t mainLoop_count;

    // set if we are driving backwards
    bool in_reverse;

    static const AP_Scheduler::Task scheduler_tasks[];

private:
    // private member functions
    void ahrs_update();
    void mount_update(void);
    void update_alt();
    void gcs_failsafe_check(void);
    void compass_accumulate(void);
    void update_compass(void);
    void update_logging1(void);
    void update_logging2(void);
    void update_aux(void);
    void one_second_loop(void);
    void update_GPS_50Hz(void);
    void update_GPS_10Hz(void);
    void update_current_mode(void);
    void update_navigation();
    void send_heartbeat(mavlink_channel_t chan);
    void send_attitude(mavlink_channel_t chan);
    void send_extended_status1(mavlink_channel_t chan);
    void send_location(mavlink_channel_t chan);
    void send_nav_controller_output(mavlink_channel_t chan);
    void send_servo_out(mavlink_channel_t chan);
    void send_radio_out(mavlink_channel_t chan);
    void send_vfr_hud(mavlink_channel_t chan);
    void send_simstate(mavlink_channel_t chan);
    void send_hwstatus(mavlink_channel_t chan);
    void send_rangefinder(mavlink_channel_t chan);
    void send_current_waypoint(mavlink_channel_t chan);
    void send_statustext(mavlink_channel_t chan);
    bool telemetry_delayed(mavlink_channel_t chan);
    void mavlink_delay_cb();
    void gcs_send_message(enum ap_message id);
    void gcs_data_stream_send(void);
    void gcs_update(void);
    void gcs_send_text_P(gcs_severity severity, const prog_char_t *str);
    void gcs_retry_deferred(void);
    bool print_log_menu(void);
    void do_erase_logs(void);
    void Log_Write_Performance();
    void Log_Write_Steering();
    void Log_Write_Startup(uint8_t type);
    void Log_Write_EntireMission();
    void Log_Write_Control_Tuning();
    void Log_Write_Nav_Tuning();
    void Log_Write_Attitude();
    void Log_Write_Sonar();
    void Log_Write_Current();
    void Log_Write_RC(void);
    void Log_Write_Baro(void);
    void Log_Read(uint16_t log_num, uint16_t start_page, uint16_t end_page);
    void start_logging() ;
    void Log_Write_Startup(uint8_t type);
    void Log_Write_EntireMission();
    void Log_Write_Current();
    void Log_Write_Nav_Tuning();
    void Log_Write_Performance();
    void Log_Write_Control_Tuning();
    void Log_Write_Sonar();
    void Log_Write_Attitude();
    void start_logging();
    void Log_Write_RC(void);
    void load_parameters(void);
    void throttle_slew_limit(int16_t last_throttle);
    bool auto_check_trigger(void);
    bool use_pivot_steering(void);
    void calc_throttle(float target_speed);
    void calc_lateral_acceleration();
    void calc_nav_steer();
    void set_servos(void);
    void set_next_WP(const struct Location& loc);
    void set_guided_WP(void);
    void init_home();
    void restart_nav();
    void exit_mission();
    void do_RTL(void);
    bool verify_RTL();
    bool verify_wait_delay();
    bool verify_within_distance();
    void do_take_picture();
    void log_picture();
    void update_commands(void);
    void delay(uint32_t ms);
    void mavlink_delay(uint32_t ms);
    uint32_t millis();
    uint32_t micros();
    void read_control_switch();
    uint8_t readSwitch(void);
    void reset_control_switch();
    void read_trim_switch();
    void update_events(void);
    void failsafe_check();
    void navigate();
    void reached_waypoint();
    void set_control_channels(void);
    void init_rc_in();
    void init_rc_out();
    void read_radio();
    void control_failsafe(uint16_t pwm);
    void trim_control_surfaces();
    void trim_radio();
    void init_barometer(void);
    void init_sonar(void);
    void read_battery(void);
    void read_receiver_rssi(void);
    void read_sonars(void);
    void report_batt_monitor();
    void report_radio();
    void report_gains();
    void report_throttle();
    void report_compass();
    void report_modes();
    void print_PID(PID * pid);
    void print_radio_values();
    void print_switch(uint8_t p, uint8_t m);
    void print_done();
    void print_blanks(int num);
    void print_divider(void);
    int8_t radio_input_switch(void);
    void zero_eeprom(void);
    void print_enabled(bool b);
    void init_ardupilot();
    void startup_ground(void);
    void set_reverse(bool reverse);
    void set_mode(enum mode mode);
    bool mavlink_set_mode(uint8_t mode);
    void failsafe_trigger(uint8_t failsafe_type, bool on);
    void startup_INS_ground(void);
    void update_notify();
    void resetPerfData(void);
    void check_usb_mux(void);
    uint8_t check_digital_pin(uint8_t pin);
    bool should_log(uint32_t mask);
    void frsky_telemetry_send(void);
    void print_hit_enter();    
};
