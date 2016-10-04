// --------------------------------------------
// Monitor incoming radio packets.
// Blinks on message or RX error.
// Sends message data to the serial port.


// --------------------------------------------

#include "stdmansos.h"
#include "../phaser_msg.h"
#include "../db_framework.h"


#define RATE_DELAY 200
#define RADIO_MAX_TX_POWER 31

// Phaser configuration message
MSG_NEW_WITH_ID(ant_msg, phaser_ping_t, PH_MSG_Test);
phaser_ping_t *ant_cfg_p = &(ant_msg.payload);

// Phaser control message(s)
MSG_NEW_WITH_ID(ctrl_msg, phaser_control_t, PH_MSG_Control);
phaser_control_t *ctrl_data_p = &(ctrl_msg.payload);

// Define a buffer for receiving messages
MSG_DEFINE_BUFFER_WITH_ID(radioBuffer, recv_data_p, RADIO_MAX_PACKET);

// Phaser test configuration message
// MSG_DEFINE_BUFFER_WITH_ID(text_msg, msg_text_data_t, PH_MSG_Text);

bool doneRecv=false;
bool captData=false;
data_record_t data_record_a[200];
int recIdx = 0;
int rxIdx=0;


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
            return;
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

        data_record_a[recIdx].rxIdx=rxIdx;
        data_record_a[recIdx].rssi=rssi;
        data_record_a[recIdx].lqi=lqi;

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
        // memcpy(text_msg.payload.text, str, len);

        radioSetTxPower(RADIO_MAX_TX_POWER);
        MSG_RADIO_SEND( *str );
    }
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
void sending_start()
{

    int i;
    for(i = 0; i < 199; i++)
    {
        char *s;
        sprintf(s,"%d\t%d\t%d\t", (int)data_record_a[i].rxIdx, (int)data_record_a[i].rssi, (int)data_record_a[i].lqi);
        send_string(s);
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
            sending_start();
        }
        led0Toggle();
    }
}
