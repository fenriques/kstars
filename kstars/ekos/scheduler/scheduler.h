/*  Ekos Scheduler Module
    Copyright (C) 2015 Jasem Mutlaq <mutlaqja@ikarustech.com>

    DBus calls from GSoC 2015 Ekos Scheduler project by Daniel Leu <daniel_mihai.leu@cti.pub.ro>

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
 */

#pragma once

#include "ui_scheduler.h"
#include "ekos/align/align.h"

#include <lilxml.h>

#include <QProcess>
#include <QTime>
#include <QTimer>
#include <QUrl>
#include <QtDBus/QtDBus>

#include <cstdint>

#include "schedulestrategy.h"

class QProgressIndicator;

class SkyObject;

namespace Ekos
{
class SequenceJob;
class SchedulerJob;
class ScheduleStrategy;

/**
 * @brief The Ekos scheduler is a simple scheduler class to orchestrate automated multi object observation jobs.
 * @author Jasem Mutlaq
 * @version 1.2
 */
class Scheduler : public QWidget, public Ui::Scheduler
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kstars.Ekos.Scheduler")

  public:
    typedef enum {
        SCHEDULER_IDLE,
        SCHEDULER_STARTUP,
        SCHEDULER_RUNNING,
        SCHEDULER_PAUSED,
        SCHEDULER_SHUTDOWN,
        SCHEDULER_ABORTED
    } SchedulerState;
    typedef enum { EKOS_IDLE, EKOS_STARTING, EKOS_STOPPING, EKOS_READY } EkosState;
    typedef enum { INDI_IDLE, INDI_CONNECTING, INDI_DISCONNECTING, INDI_PROPERTY_CHECK, INDI_READY } INDIState;
    typedef enum {
        STARTUP_IDLE,
        STARTUP_SCRIPT,
        STARTUP_UNPARK_DOME,
        STARTUP_UNPARKING_DOME,
        STARTUP_UNPARK_MOUNT,
        STARTUP_UNPARKING_MOUNT,
        STARTUP_UNPARK_CAP,
        STARTUP_UNPARKING_CAP,
        STARTUP_ERROR,
        STARTUP_COMPLETE
    } StartupState;
    typedef enum {
        SHUTDOWN_IDLE,
        SHUTDOWN_PARK_CAP,
        SHUTDOWN_PARKING_CAP,
        SHUTDOWN_PARK_MOUNT,
        SHUTDOWN_PARKING_MOUNT,
        SHUTDOWN_PARK_DOME,
        SHUTDOWN_PARKING_DOME,
        SHUTDOWN_SCRIPT,
        SHUTDOWN_SCRIPT_RUNNING,
        SHUTDOWN_ERROR,
        SHUTDOWN_COMPLETE
    } ShutdownState;
    typedef enum {
        PARKWAIT_IDLE,
        PARKWAIT_PARK,
        PARKWAIT_PARKING,
        PARKWAIT_PARKED,
        PARKWAIT_UNPARK,
        PARKWAIT_UNPARKING,
        PARKWAIT_UNPARKED,
        PARKWAIT_ERROR
    } ParkWaitStatus;

    /** @brief Columns, in the same order as UI. */
    typedef enum {
        SCHEDCOL_NAME = 0,
        SCHEDCOL_STATUS,
        SCHEDCOL_CAPTURES,
        SCHEDCOL_SCORE,
        SCHEDCOL_STARTTIME,
        SCHEDCOL_ENDTIME,
        SCHEDCOL_DURATION,
        SCHEDCOL_COUNT
    } SchedulerColumns;

    Scheduler();
    ~Scheduler() = default;

    QString getCurrentJobName();
    void appendLogText(const QString &);
    QString getLogText() { return logText.join("\n"); }
    void clearLog();

    void addObject(SkyObject *object);

    /**
         * @brief startSlew DBus call for initiating slew
         */
    void startSlew();

    /**
         * @brief startFocusing DBus call for feeding ekos the specified settings and initiating focus operation
         */
    void startFocusing();

    /**
         * @brief startAstrometry initiation of the capture and solve operation. We change the job state
         * after solver is started
         */
    void startAstrometry();

    /**
         * @brief startGuiding After ekos is fed the calibration options, we start the guiging process
         * @param resetCalibration By default calibration is not reset until it is explicitly requested
         */
    void startGuiding(bool resetCalibration = false);

    /**
         * @brief startCapture The current job file name is solved to an url which is fed to ekos. We then start the capture process
         */
    void startCapture();

    /**
         * @brief getNextAction Checking for the next appropriate action regarding the current state of the scheduler  and execute it
         */
    void getNextAction();

    /**
         * @brief disconnectINDI disconnect all INDI devices from server.
         */
    void disconnectINDI();

    /**
         * @brief stopEkos shutdown Ekos completely
         */
    void stopEkos();

    /**
         * @brief stopGuiding After guiding is done we need to stop the process
         */
    void stopGuiding();

    /**
         * @brief setSolverAction set the GOTO mode for the solver
         * @param mode 0 For Sync, 1 for SlewToTarget, 2 for Nothing
         */
    void setSolverAction(Align::GotoMode mode);

    /** @defgroup SchedulerDBusInterface Ekos DBus Interface - Scheduler Module
         * Ekos::Align interface provides primary functions to run and stop the scheduler.
        */

    /*@{*/

    /** DBUS interface function.
         * @brief Start the scheduler main loop and evaluate jobs and execute them accordingly.
         */
    Q_SCRIPTABLE Q_NOREPLY void start();

    /** DBUS interface function.
         * @brief Stop the scheduler.
         */
    Q_SCRIPTABLE Q_NOREPLY void stop();

    /** DBUS interface function.
         * @brief Loads the Ekos Scheduler List (.esl) file.
         * @param fileURL path to a file
         * @return true if loading file is successful, false otherwise.
         */
    Q_SCRIPTABLE bool loadScheduler(const QString &fileURL);

    /** DBUS interface function.
         * @brief Resets all jobs to IDLE
         */
    Q_SCRIPTABLE void resetAllJobs();

    /** @}*/

  protected slots:

    /**
         * @brief select object from KStars's find dialog.
         */
    void selectObject();

    /**
         * @brief Selects FITS file for solving.
         */
    void selectFITS();

    /**
         * @brief Selects sequence queue.
         */
    void selectSequence();

    /**
         * @brief Selects sequence queue.
         */
    void selectStartupScript();

    /**
         * @brief Selects sequence queue.
         */
    void selectShutdownScript();

    /**
         * @brief addToQueue Construct a SchedulerJob and add it to the queue or save job settings from current form values.
         * jobUnderEdit determines whether to add or edit
         */
    void saveJob();

    /**
         * @brief addJob Add a new job from form values
         */
    void addJob();

    /**
         * @brief editJob Edit an observation job
         * @param i index model in queue table
         */
    void loadJob(QModelIndex i);

    /**
         * @brief removeJob Remove a job from the currently selected row. If no row is selected, it remove the last job in the queue.
         */
    void removeJob();

    void toggleScheduler();
    void pause();
    void save();
    void saveAs();
    void load();

    void resetJobState(QModelIndex i);

    void resetJobEdit();

    /**
         * @brief checkJobStatus Check the overall state of the scheduler, Ekos, and INDI. When all is OK, it call evaluateJobs();
         */
    void checkStatus();

    /**
         * @brief checkJobStage Check the progress of the job states and make DBUS call to start the next stage until the job is complete.
         */
    void checkJobStage();

    /**
         * @brief findNextJob Check if the job met the completion criteria, and if it did, then it search for next job candidate. If no jobs are found, it starts the shutdown stage.
         */
    void findNextJob();

    /**
         * @brief stopCurrentJobAction Stop whatever action taking place in the current job (eg. capture, guiding...etc).
         */
    void stopCurrentJobAction();

    /**
         * @brief readProcessOutput read running script process output and display it in Ekos
         */
    void readProcessOutput();

    /**
         * @brief checkProcessExit Check script process exist status. This is called when the process exists either normally or abnormally.
         * @param exitCode exit code from the script process. Depending on the exist code, the status of startup/shutdown procedure is set accordingly.
         */
    void checkProcessExit(int exitCode);

    /**
         * @brief watchJobChanges Watch any changes in form values and apply changes to current job selection or ignore any changes
         * @param enable True to watch changes and apply them to current job, false to ignore changes
         */
    void watchJobChanges(bool enable);
    /**
         * @brief setDirty Call it to mark the Ekos Scheduler List for change. Next time save button is invoked, the complete content is written to disk.
         */
    void setDirty();

    /**
         * @brief resumeCheckStatus If the scheduler primary loop was suspended due to weather or sleep event, resume it again.
         */
    void resumeCheckStatus();

    /**
         * @brief checkWeather Check weather status and act accordingly depending on the current status of the scheduler and running jobs.
         */
    void checkWeather();

    /**
         * @brief wakeUpScheduler Wake up scheduler from sleep state
         */
    void wakeUpScheduler();

    /**
         * @brief startJobEvaluation Start job evaluation only without starting the scheduler process itself. Display the result to the user.
         */
    void startJobEvaluation();

    /**
         * @brief startMosaicTool Start Mosaic tool and create jobs if necessary.
         */
    void startMosaicTool();

    /**
         * @brief displayTwilightWarning Display twilight warning to user if it is unchecked.
         */
    void checkTwilightWarning(bool enabled);

    void runStartupProcedure();
    void checkStartupProcedure();

    void runShutdownProcedure();
    void checkShutdownProcedure();

  signals:
    void newLog();
    void weatherChanged(IPState state);
    void newTarget(const QString &);

  private:
    /**
         * @brief evaluateJobs evaluates the current state of each objects and gives each one a score based on the constraints.
         * Given that score, the scheduler will decide which is the best job that needs to be executed.
         */
    void evaluateJobs();

    /**
         * @brief executeJob After the best job is selected, we call this in order to start the process that will execute the job.
         * checkJobStatus slot will be connected in order to figure the exact state of the current job each second
         * @param value
         */
    void executeJob(SchedulerJob *job);

    void executeScript(const QString &filename);


    /**
         * @brief calculateDawnDusk Get dawn and dusk times for today
         */
    void calculateDawnDusk();

    /**
         * @brief checkEkosState Check ekos startup stages and take whatever action necessary to get Ekos up and running
         * @return True if Ekos is running, false if Ekos start up is in progress.
         */
    bool checkEkosState();

    /**
         * @brief checkINDIState Check INDI startup stages and take whatever action necessary to get INDI devices connected.
         * @return True if INDI devices are connected, false if it is under progress.
         */
    bool checkINDIState();

    /**
         * @brief checkStartupState Check startup procedure stages and make sure all stages are complete.
         * @return True if startup is complete, false otherwise.
         */
    bool checkStartupState();

    /**
         * @brief checkShutdownState Check shutdown procedure stages and make sure all stages are complete.
         * @return
         */
    bool checkShutdownState();

    /**
         * @brief checkParkWaitState Check park wait state.
         * @return If parking/unparking in progress, return false. If parking/unparking complete, return true.
         */
    bool checkParkWaitState();

    /**
         * @brief parkMount Park mount
         */
    void parkMount();

    /**
         * @brief unParkMount Unpark mount
         */
    void unParkMount();

    /**
         * @return True if mount is parked
         */
    bool isMountParked();

    /**
         * @brief parkDome Park dome
         */
    void parkDome();

    /**
         * @brief unParkDome Unpark dome
         */
    void unParkDome();

    /**
         * @return True if dome is parked
         */
    bool isDomeParked();

    /**
         * @brief parkCap Close dust cover
         */
    void parkCap();

    /**
         * @brief unCap Open dust cover
         */
    void unParkCap();

    /**
         * @brief checkMountParkingStatus check mount parking status and updating corresponding states accordingly.
         */
    void checkMountParkingStatus();

    /**
         * @brief checkDomeParkingStatus check dome parking status and updating corresponding states accordingly.
         */
    void checkDomeParkingStatus();

    /**
         * @brief checkDomeParkingStatus check dome parking status and updating corresponding states accordingly.
         */
    void checkCapParkingStatus();

    /**
         * @brief saveScheduler Save scheduler jobs to a file
         * @param path path of a file
         * @return true on success, false on failure.
         */
    bool saveScheduler(const QUrl &fileURL);

    /**
         * @brief processJobInfo Process the job information from a scheduler file and populate jobs accordingly
         * @param root XML root element of JOB
         * @return true on success, false on failure.
         */
    bool processJobInfo(XMLEle *root);

    /**
         * @brief createJobSequence Creates a job sequence for the mosaic tool given the prefix and output dir. The currently selected sequence file is modified
         * and a new version given the supplied parameters are saved to the output directory
         * @param prefix Prefix to set for the job sequence
         * @param outputDir Output dir to set for the job sequence
         * @return True if new file is saved, false otherwise
         */
    bool createJobSequence(XMLEle *root, const QString &prefix, const QString &outputDir);

    /** @internal Change the current job, updating associated widgets.
     * @param job is an existing SchedulerJob to set as current, or nullptr.
     */
    void setCurrentJob(SchedulerJob *job);

    void loadProfiles();

    XMLEle *getSequenceJobRoot();

    Ekos::Scheduler *ui { nullptr };
    //DBus interfaces
    QDBusInterface *focusInterface { nullptr };
    QDBusInterface *ekosInterface { nullptr };
    QDBusInterface *captureInterface { nullptr };
    QDBusInterface *mountInterface { nullptr };
    QDBusInterface *alignInterface { nullptr };
    QDBusInterface *guideInterface { nullptr };
    QDBusInterface *domeInterface { nullptr };
    QDBusInterface *weatherInterface { nullptr };
    QDBusInterface *capInterface { nullptr };
    // Scheduler and job state and stages
    SchedulerState state { SCHEDULER_IDLE };
    EkosState ekosState { EKOS_IDLE };
    INDIState indiState { INDI_IDLE };
    StartupState startupState { STARTUP_IDLE };
    ShutdownState shutdownState { SHUTDOWN_IDLE };
    ParkWaitStatus parkWaitState { PARKWAIT_IDLE };
    /// Active job
    SchedulerJob *currentJob { nullptr };
    /// scheduling strategy
    ScheduleStrategy *strategy { nullptr };
    /// URL to store the scheduler file
    QUrl schedulerURL;
    /// URL for Ekos Sequence
    QUrl sequenceURL;
    /// FITS URL to solve
    QUrl fitsURL;
    /// Startup script URL
    QUrl startupScriptURL;
    /// Shutdown script URL
    QUrl shutdownScriptURL;
    /// Store all log strings
    QStringList logText;
    /// Busy indicator widget
    QProgressIndicator *pi { nullptr };
    /// Are we editing a job right now? Job row index
    int jobUnderEdit { -1 };
    /// Pointer to Geograpic locatoin
    GeoLocation *geo { nullptr };
    /// How many repeated job batches did we complete thus far?
    uint16_t captureBatch { 0 };
    /// Startup and Shutdown scripts process
    QProcess scriptProcess;
    /// Was job modified and needs saving?
    bool mDirty { false };
    /// Keep track of how many times we didn't receive weather updates
    uint8_t noWeatherCounter { 0 };
    /// Are we shutting down until later?
    bool preemptiveShutdown { false };
    /// Only run job evaluation
    bool jobEvaluationOnly { false };
    /// Keep track of Load & Slew operation
    bool loadAndSlewProgress { false };
    /// Check if initial autofocus is completed and do not run autofocus until there is a change is telescope position/alignment.
    bool autofocusCompleted { false };
    /// Keep track of INDI connection failures
    uint8_t indiConnectFailureCount { 0 };
    /// Keep track of Ekos focus module failures
    uint8_t focusFailureCount { 0 };
    /// Keep track of Ekos guide module failures
    uint8_t guideFailureCount { 0 };
    /// Keep track of Ekos align module failures
    uint8_t alignFailureCount { 0 };
    /// Keep track of Ekos capture module failures
    uint8_t captureFailureCount { 0 };
    /// Call checkWeather when weatherTimer time expires. It is equal to the UpdatePeriod time in INDI::Weather device.
    QTimer weatherTimer;
    /// Timer to put the scheduler into sleep mode until a job is ready
    QTimer sleepTimer;
    /// To call checkStatus
    QTimer schedulerTimer;
    /// To call checkJobStage
    QTimer jobTimer;

    /// Generic time to track timeout of current operation in progress
    QTime currentOperationTime;

    QUrl dirPath;
};
}
