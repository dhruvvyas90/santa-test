// --------------------------------------------
// Monitor incoming radio packets.
// Blinks on message or RX error.
// Sends message data to the serial port.


// --------------------------------------------

#include "stdmansos.h"
#include "../phaser_msg.h"
#include "../db_framework.h"


#define RATE_DELAY 200

// Phaser configuration message
MSG_NEW_WITH_ID(ant_msg, phaser_ping_t, PH_MSG_Test);
phaser_ping_t *ant_cfg_p = &(ant_msg.payload);

// Phaser control message(s)
MSG_NEW_WITH_ID(ctrl_msg, phaser_control_t, PH_MSG_Control);
phaser_control_t *ctrl_data_p = &(ctrl_msg.payload);

// Define a buffer for receiving messages
MSG_DEFINE_BUFFER_WITH_ID(radioBuffer, recv_data_p, RADIO_MAX_PACKET);

bool doneRecv=false;
bool captData=false;
data_record_t data_record_a[200];
int recIdx = 0;
int rxIdx=0;

// -------------------------------------------------------------------------
// Setup the phaser
// -------------------------------------------------------------------------
void phaser_init()
{
    // Init the test infrastructure
    lastAngle = ANGLE_NOT_SET_VALUE;    // Force stepper angle recalibration
    set_angle( -20 );
    set_angle( 0 );
    
    // Init the iterators
    testIdx.power.idx = 0;
    testIdx.angle.idx = 0;
    testIdx.angle.limit = test_config.angle_count;
    
    // Init the antena configuration
    ant_cfg_p->expIdx = 0;
    ant_cfg_p->msgCounter = 0;
    ant_cfg_p->angle = 0;
    ant_cfg_p->power = test_config.power[0];
    
    ant_test_init(&testIdx, &test_config, ant_cfg_p);
}

// --------------------------------------------

// Setup the default test run parameters
test_config_t test_config = {
    .platform_id=PH_PHASER,
    .start_delay=1000,
    .send_count=100,
    .send_delay=20,
    .angle_step=25,
    .angle_count=8,
    .power={31,23,15,0},
    .ant.phaseA={
        .start=0,
        .step=32,
        .count=0    // 0
    },
    .ant.phaseB={
        .start=0,
        .step=32,
        .count=8    // 8
    },
};



// Prototypes
void send_ctrl_msg(msg_action_t act);


// --------------------------------------------
// --------------------------------------------
void send_ctrl_msg(msg_action_t act)
{
    ctrl_msg.payload.action = act;
    MSG_DO_CHECKSUM( ctrl_msg );
    MSG_RADIO_SEND( ctrl_msg );
}




// --------------------------------------------
// --------------------------------------------
void onRadioRecv(void)
{
    static bool flRxProcessing=false;
    if(flRxProcessing){
        return;
    }
    flRxProcessing=true;    // There is a chance for a small race condition
    bool flOK=true;
    
    //Control payload.
    MSG_NEW_PAYLOAD_PTR(radioBuffer, phaser_control_t, control_p);
    MSG_NEW_PAYLOAD_PTR(radioBuffer, phaser_ping_t, test_data_p);
    
    if (radioBuffer.id == PH_MSG_Control) {
        if (control_p->action == MSG_ACT_DONE) {
            doneRecv = true;
        }
    }
    
    if (radioBuffer.id == PH_MSG_Test) {
        MSG_CHECK_FOR_PAYLOAD(radioBuffer, phaser_ping_t, flOK=false );
        if( !flOK ){
            break;
        }
        captData=true;
    }
    
    
    if (captData) {
        
        int16_t rxLen;
        rssi_t rssi;
        lqi_t lqi;
        
        rxIdx++;
        if( rxIdx < 0 ) rxIdx=0;
        
        
        led1Toggle();
        
        if( ! MSG_SIGNATURE_OK(radioBuffer) ) {
            flRxProcessing = false;
            return;
        }
        
        rxLen = radioRecv(&radioBuffer, sizeof(radioBuffer));
        
        if (rxLen < 0) {
            led2Toggle();
            flRxProcessing=false;
            return;
        }
        
        rssi = radioGetLastRSSI();
        lqi = radioGetLastLQI();
        
        data_record_a[recIdx]->rxIdx=rxIdx;
        data_record_a[recIdx]->rssi=rssi;
        data_record_a[recIdx]->lqi=lqi;
        
        recIdx++;
        captData=false;
    }
    
    flRxProcessing=false;
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
void send_string(char *str)
{
    int len = strlen(str);
    if(len>0 && len<MSG_TEXT_SIZE_MAX-1){
        memcpy(text_msg.payload.text, str, len);
        
        radioSetTxPower(RADIO_MAX_TX_POWER);
        MSG_RADIO_SEND( text_msg );
    }
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
void sending_start()
{
    
    for(i = 0; i < 199; i++)
    {
        send_string("%d\t%d\t%d\t", (int)data_record_a[i]->rxIdx, (int)data_record_a[i]->rssi, (int)data_record_a[i]->lqi);
    }
    
    send_ctrl_msg(MSG_ACT_DONE);
}

// --------------------------------------------
// --------------------------------------------
void appMain(void)
{
    
    radioSetReceiveHandle(onRadioRecv);
    radioOn();
    mdelay(200);
    
    while (1) {
        if (doneRecv) {
            led1Toggle();
            phaser_init();
            sending_start();
        }
        led0Toggle();
    }
}
