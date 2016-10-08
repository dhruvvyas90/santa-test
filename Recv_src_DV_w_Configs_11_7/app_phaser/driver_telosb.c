// --------------------------------------------
// Phaser node: digitally controlled beam forming
// The driver.
// Compile target: telosb
// --------------------------------------------

#include "stdmansos.h"
#include "digital.h"

#include "../phaser_msg.h"

#include "antenna_driver.h"

// Hardware definitions
PIN_DEFINE(TmoteBtn, 2, 7);

// -------------------------------------------------------------------------
// Driver name and ID
// -------------------------------------------------------------------------
char *ant_driver_name = "TelosB";
#define PLATFORM_ID  PH_TELOSB

#define NO_OF_PHASEA 8
#define NO_OF_PHASEB 8
#define PHASEA_INCREMENTAL 32
#define PHASEB_INCREMENTAL 32

uint32_t timeRand;
uint8_t rand1;
uint8_t phase_a_data[NO_OF_PHASEA];
uint8_t ant_a_phase[NO_OF_PHASEA];
uint8_t phase_b_data[NO_OF_PHASEB];
uint8_t ant_b_phase[NO_OF_PHASEB];
uint8_t rand_a_count = NO_OF_PHASEA;
uint8_t rand_b_count = NO_OF_PHASEB;

long holdrand;



// -------------------------------------------------------------------------
// Set of test configurations that should be executed
// -------------------------------------------------------------------------
test_config_t testSet[] = {
    {
        .platform_id = PLATFORM_ID,     // Short test
        .start_delay = 100,
        .send_delay  = 1,
        .send_count  = 9,
        .angle_step  = 0,
        .angle_count = 0,
        .ant.phaseA.start = 0,
        .ant.phaseA.step  = 32,
        .ant.phaseA.count = 8,
        .ant.phaseB.start = 0,
        .ant.phaseB.step  = 32,
        .ant.phaseB.count = 8,
        .power = {7, 0}
    },
    // {
    //     .platform_id = PLATFORM_ID,     // Longer test
    //     .start_delay = 1000,
    //     .send_delay  = 5,
    //     .send_count  = 100,
    //     .angle_step  = 5,
    //     .angle_count = 40,
    //     .ant.phaseA.count = 0,
    //     .ant.phaseB.count = 0,
    //     .power = {31, 23, 15, 7, 3, 0}
    // }
};
const size_t testSet_size = sizeof(testSet)/sizeof(testSet[0]);


// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
void ant_driver_init()
{
    //Init Button
    TmoteBtnAsInput();      \
    return;
}


// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
bool ant_test_sanity_check(test_config_t *newTest)
{
    return true;
}

//random functions

void o_seed (unsigned int seed)
{
    holdrand = (long)seed;
}

int o_rand (void)
{
    return(((holdrand = holdrand * 214013 + 2531011) >> 16) & 0x7fff);
}

void ant_test_init(test_loop_t *testIdx, test_config_t *test_config, phaser_ping_t *ant_cfg_p)
{
    int i;
    int j;
    //o_seed(1234);
    // Init the iterators
    testIdx->phaseA.idx = 0;
    testIdx->phaseA.limit = test_config->ant.phaseA.count;
    testIdx->phaseB.idx = 0;
    testIdx->phaseB.limit = test_config->ant.phaseB.count;
    rand_a_count = NO_OF_PHASEA;
    rand_b_count = NO_OF_PHASEB;
    //timeRand = getTimeMs();
    rand1 = o_rand() % rand_a_count;
    for(i=0;i<NO_OF_PHASEA;i++)
    {
      ant_a_phase[i] = i * PHASEA_INCREMENTAL;
    }
    for(i=0;i<NO_OF_PHASEB;i++)
    {
      ant_b_phase[i] = i * PHASEB_INCREMENTAL;
    }
    //rand2 = timeRand % rand_b_count;
    // Init the antena configuration
    ant_cfg_p->ant.phaseA = ant_a_phase[rand1];
    j=0;
    for(i=0;i<rand_a_count;i++)
    {
      if(rand1 == i)
      {
        continue;
      }
      ant_a_phase[j++] = ant_a_phase[i];
    }
    rand_a_count--;
    //ant_cfg_p->ant.phaseB = test_config->ant.phaseB.start;
    //timeRand = getTimeMs();
    rand1 = o_rand() % rand_b_count;
    ant_cfg_p->ant.phaseB = ant_b_phase[rand1];
    j=0;
    for(i=0;i<rand_b_count;i++)
    {
      if(rand1 == i)
      {
        continue;
      }
      ant_b_phase[j++] = ant_b_phase[i];
    }
    rand_b_count--;
    //ant_cfg_p->ant.phaseA = 0;
    //ant_cfg_p->ant.phaseB = 0;
}



// -------------------------------------------------------------------------
// Setup the test run
// Return true when next iteration is ready
// Return false when done (no more iterations possible)
// -------------------------------------------------------------------------
bool ant_test_next_config(test_loop_t *testIdx, test_config_t *test_config, phaser_ping_t *ant_cfg_p)
{
    uint8_t i;
    uint8_t j;
    // Next phase
    if(test_config->ant.phaseA.count){
        //timeRand = getTimeMs();
        rand1 = o_rand() % rand_b_count;
        testIdx->phaseA.idx++;
        if( testIdx->phaseA.idx >= testIdx->phaseA.limit ){
            testIdx->phaseA.idx = 0;
            rand_a_count = NO_OF_PHASEA;
            for(i=0;i<NO_OF_PHASEA;i++)
            {
              ant_a_phase[i] = i * PHASEA_INCREMENTAL;
            }
            ant_cfg_p->ant.phaseA = ant_a_phase[rand1];
            j=0;
            for(i=0;i<rand_a_count;i++)
            {
              if(rand1 != i)
              {
                ant_a_phase[j] = ant_a_phase[i];
                j++;
              }
            }
            rand_a_count--;
            //ant_cfg_p->ant.phaseA = 0;
        }
        else {
            //timeRand = getTimeMs();
            rand1 = o_rand() % rand_a_count;
            ant_cfg_p->ant.phaseA = ant_a_phase[rand1];
            j=0;
            for(i=0;i<rand_a_count;i++)
            {
              if(rand1 != i)
              {
                ant_a_phase[j] = ant_a_phase[i];
                j++;
              }
            }
            rand_a_count--;
            //ant_cfg_p->ant.phaseA = 0;
            return true;
        }
    }
    // if(test_config->ant.phaseB.count){
    //     testIdx->phaseB.idx++;
    //     if( testIdx->phaseB.idx >= testIdx->phaseB.limit ){
    //         testIdx->phaseB.idx = 0;
    //         ant_cfg_p->ant.phaseB = test_config->ant.phaseB.start;
    //     }
    //     else {
    //         ant_cfg_p->ant.phaseB += test_config->ant.phaseB.step;
    //         return true;
    //     }
    // }
    if(test_config->ant.phaseB.count)
    {
        //timeRand = getTimeMs();
        rand1 = o_rand() % rand_b_count;
        testIdx->phaseB.idx++;
        if( testIdx->phaseB.idx >= testIdx->phaseB.limit )
        {
            testIdx->phaseB.idx = 0;
            rand_b_count = NO_OF_PHASEB;
            for(i=0;i<NO_OF_PHASEB;i++)
            {
              ant_b_phase[i] = i * PHASEB_INCREMENTAL;
            }
            ant_cfg_p->ant.phaseB = ant_b_phase[rand1];
            j=0;
            for(i=0;i<rand_b_count;i++)
            {
              if(rand1 != i)
              {
                ant_b_phase[j] = ant_b_phase[i];
                j++;
              }
            }
            rand_b_count--;
            //ant_cfg_p->ant.phaseB = 0;
        }
        else
        {
            ant_cfg_p->ant.phaseB = ant_b_phase[rand1];
            j=0;
            for(i=0;i<rand_b_count;i++)
            {
              if(rand1 != i)
              {
                ant_b_phase[j] = ant_b_phase[i];
                j++;
              }
            }
            rand_b_count--;
            //ant_cfg_p->ant.phaseB = 0;
            return true;
        }
    }
    return false;
}


// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
void ant_test_setup(phaser_ping_t *ant_cfg_p)
{
    //Nothing to do
    return;
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
void ledBtnDown()
{
    int i;
    for (i=0; i<3; i++)
    {
        ledOn();
        mdelay(100);
        ledOff();
        mdelay(100);
    }
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
bool ant_check_button()
{
    if( TmoteBtnRead() )   return false;
    ledBtnDown();
    while( !TmoteBtnRead() ){   // Wait untill button released
        ledBtnDown();
    }

    return true;
}
