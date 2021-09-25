/*
   The Cloud Game
   by Move38

   design by: Daniel King, Jude Pinto
   development by: Jonathan Bobrow

   Press a Blink to switch on or off, also switches neighbors.
   Turn all Blinks on to win.
   Triple-click to reset.

*/

#define MAX_DISTANCE 16

// comms
#define FLIP  MAX_DISTANCE + 1
#define RESET MAX_DISTANCE + 2
#define ACK   MAX_DISTANCE + 3

byte myCommState = MAX_DISTANCE;

bool isOn = false;
bool wasClicked = false;
bool shouldReturn = false;
bool flipReceived = false;
byte flipFace = FACE_COUNT;

Timer resetTimer;
#define RESET_DURATION 500

Timer slowTimer;
#define DELAY 500

bool firstFromWake = false;

/*
   WIN CONDITION ANIMATION VARIABLES
*/
//Silver Lining Constants for delta
#define SPARKLE_DURATION        800
#define SPARKLE_CYCLE_DURATION  1600


//Sun Spot Values
byte sunSpot_hue = 30;
Timer sunSpotTimer;
Timer sunSpotDelayTimer;
byte sunspotFace = FACE_COUNT;

uint32_t timeOfWinCondition = 0;

/*
   WIN CONDITION ANIMATION
*/


void setup() {
  // nothing to do here
  randomize();
}


void loop() {

  //  if (slowTimer.isExpired()) {
  //    slowTimer.set(DELAY);
  /*
     INPUTS
  */

  // dump input from wake
  if (hasWoken()) {
    firstFromWake = true;
  }

  if ( buttonReleased() && firstFromWake )
  {
    buttonSingleClicked();
    firstFromWake = false;
  }

  if ( buttonSingleClicked() ) {
    // flip
    wasClicked = true;
  }


  if ( buttonMultiClicked() ) {
    // reset
    resetTimer.set(RESET_DURATION);
    isOn = false;
  }


  /*
     NEIGHBOR INTERACTIONS
  */

  // propogate inverse of distance from OFF blink
  byte inverseDistanceFromOff = 0;

  // if it is not on, then we are the OFF blink, and the inverse distance is MAX_DISTANCE
  if (!isOn) {
    inverseDistanceFromOff = MAX_DISTANCE;
  }

  // now search neighbors for closest to OFF blink
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {

      byte neighborVal = getLastValueReceivedOnFace(f);

      // handle neighbor message if it is NOT in the distance range

      // handle flip
      if ( neighborVal == FLIP ) {
        myCommState = ACK;
        flipReceived = true;
        flipFace = f;
        break;  // get out of this for loop
      }
      else if ( neighborVal != FLIP && flipFace == f ) {
        if ( flipReceived ) {
          isOn = !isOn;
          myCommState = MAX_DISTANCE;
          flipReceived = false;
          flipFace = FACE_COUNT;
          break;  // get out of this for loop
        }
        // done receiving
      }

      if ( neighborVal == RESET ) {
        if ( resetTimer.isExpired() ) {
          resetTimer.set( RESET_DURATION );
          isOn = false;
          break;  // get out of this for loop
        }
      }

      if ( neighborVal <= MAX_DISTANCE && neighborVal >= inverseDistanceFromOff ) {
        if (neighborVal > 0) {
          inverseDistanceFromOff = neighborVal - 1;
        }
        else {
          inverseDistanceFromOff = neighborVal;
        }
        myCommState = inverseDistanceFromOff;
      }
    }
  }

  if ( wasClicked ) {
    if ( areAllNeighbors( ACK ) ) {
      myCommState = ACK;
      wasClicked = false;
      shouldReturn = true;
      isOn = !isOn;

    }
    else {
      myCommState = FLIP;
    }
  }

  if ( shouldReturn ) {
    if ( !areAnyNeighbors( ACK ) ) {
      shouldReturn = false;
      myCommState = MAX_DISTANCE;
    }
  }

  if ( !resetTimer.isExpired() ) {
    if ( resetTimer.getRemaining() > RESET_DURATION / 2 ) {
      myCommState = RESET;
    }
    else {
      myCommState = MAX_DISTANCE;
    }
  }

  /*
      COMMUNICATE
  */

  setValueSentOnAllFaces(myCommState);


  /*
     DISPLAY
  */

  if ( isOn ) {
    setColor(WHITE);
  }
  else {
    byte groupAbri = 64 + sin8_C( millis() / 8 ) / 4;
    byte groupBbri = 64 + sin8_C( 128 + (millis() / 8) ) / 4;
    setColorOnFace(makeColorHSB(160, 255, groupAbri), 0);
    setColorOnFace(makeColorHSB(160, 255, groupBbri), 1);
    setColorOnFace(makeColorHSB(160, 255, groupAbri), 2);
    setColorOnFace(makeColorHSB(160, 255, groupBbri), 3);
    setColorOnFace(makeColorHSB(160, 255, groupAbri), 4);
    setColorOnFace(makeColorHSB(160, 255, groupBbri), 5);
  }

  if ( myCommState == 0 ) {
    // display win
    displayWin();
  }
  else {
    timeOfWinCondition = millis();
  }

  if ( !resetTimer.isExpired() ) {
    // display reset
    setColor( dim( WHITE, map(resetTimer.getRemaining(), 0, RESET_DURATION, 0, 255) ) );
  }

  /*
     DEBUG DISPLAY
  */
  //  switch ( myCommState ) {
  //    case FLIP:  setColorOnFace( BLUE,  0 );   break;
  //    case ACK:   setColorOnFace( GREEN, 0 );   break;
  //    case RESET: setColorOnFace( RED,   0 );   break;
  //    default:    setColorOnFace( YELLOW, 0);   break;
  //  }
  //  }
}


bool areAllNeighbors(byte val) {

  FOREACH_FACE(f) {
    if ( !isValueReceivedOnFaceExpired(f) ) {
      if ( getLastValueReceivedOnFace(f) != val ) {
        // found one that doesn't match
        return false;
      }
    }
  }

  // looks like all connected are the val
  return true;
}

bool areAnyNeighbors(byte val) {

  FOREACH_FACE(f) {
    if ( !isValueReceivedOnFaceExpired(f) ) {
      if ( getLastValueReceivedOnFace(f) == val ) {
        // found one that matches
        return true;
      }
    }
  }

  // looks like all connected are NOT the val
  return false;
}

/*
   WIN ANIMATION - REDUX
*/

#define WIN_ANI_STAGE_1_DURATION 1000
#define WIN_ANI_STAGE_2_DURATION 1000
#define WIN_ANI_STAGE_3_DURATION 1000

#define SUNSPOT_DURATION 1000

#define END_STAGE_1     WIN_ANI_STAGE_1_DURATION
#define START_STAGE_2   WIN_ANI_STAGE_1_DURATION
#define END_STAGE_2     WIN_ANI_STAGE_1_DURATION + WIN_ANI_STAGE_2_DURATION
#define START_STAGE_3   WIN_ANI_STAGE_1_DURATION + WIN_ANI_STAGE_2_DURATION
#define END_STAGE_3     WIN_ANI_STAGE_1_DURATION + WIN_ANI_STAGE_2_DURATION + WIN_ANI_STAGE_3_DURATION


void displayWin() {

  uint32_t timeSinceWin = millis() - timeOfWinCondition;

  // Stage 1 - fade down
  if ( timeSinceWin < END_STAGE_1 ) {

    byte bri = 255 - map(timeSinceWin, 0, WIN_ANI_STAGE_1_DURATION, 0, 255);
    setColor(dim(WHITE, bri));

  }
  // Stage 2 - fade up border
  else if ( timeSinceWin >= START_STAGE_2 && timeSinceWin < END_STAGE_2 ) {

    setColor(OFF);

    FOREACH_FACE(f) {
      if (isValueReceivedOnFaceExpired(f)) { // only the borders
        byte bri;
        byte chance = map(timeSinceWin, START_STAGE_2, END_STAGE_3, 12, 36);
        bool on = random(chance) > 0;
        if (on) {
          bri = map(timeSinceWin, START_STAGE_2, END_STAGE_2, 0, 255);
          setColorOnFace(dim(WHITE, bri), f);
        }
        else {
          bri = 255 - map(timeSinceWin, START_STAGE_2, END_STAGE_3, 0, 255);
          setColorOnFace(dim(WHITE, bri), f);
        }
      }
    }
  }
  // Stage 3 - fade up center
  else if ( timeSinceWin >= START_STAGE_3 && timeSinceWin < END_STAGE_3 ) {

    setColor(OFF);

    FOREACH_FACE(f) {
      if (isValueReceivedOnFaceExpired(f)) { // only the borders
        byte chance = map(timeSinceWin, START_STAGE_2, END_STAGE_3, 12, 36);
        bool on = random(chance) > 0;
        if (on) {
          setColorOnFace(WHITE, f);
        }
        else {
          byte bri = map(timeSinceWin, START_STAGE_3, END_STAGE_3, 0, 255);
          setColorOnFace(dim(WHITE, bri), f);
        }
      }
      else {
        byte bri = map(timeSinceWin, START_STAGE_3, END_STAGE_3, 0, 255);
        setColorOnFace(makeColorHSB(160, 255, bri), f);
      }
    }

  }
  // Stage 4 - loop the center sun spots
  else {
    setColor(OFF);

    // choose sun spot location
    if (sunSpotTimer.isExpired()) {
      if (sunSpotDelayTimer.isExpired()) {
        uint16_t delayTime = 1000 + random(3000);
        sunSpotDelayTimer.set(delayTime);
        sunspotFace = random(5);
        sunSpotTimer.set(SUNSPOT_DURATION);
      }
    }

    FOREACH_FACE(f) {
      if (isValueReceivedOnFaceExpired(f)) { // only the borders
        setColorOnFace(WHITE, f);
      }
      else {
        // setColorOnFace(makeColorHSB(160, 255, 255), f);

        // sunspots here
        if ( !sunSpotTimer.isExpired() && sunspotFace == f) {
          
          byte hue, sat, bri;

          if ( sunSpotTimer.getRemaining() > SUNSPOT_DURATION/2 ) {
            hue = sunSpot_hue;
            sat =  map( sunSpotTimer.getRemaining(), SUNSPOT_DURATION/2, SUNSPOT_DURATION, 0, 255);
            bri = map( sunSpotTimer.getRemaining(), SUNSPOT_DURATION/2, SUNSPOT_DURATION, 0, 255);
          }
          else {
            hue = 160;
            sat = 255 - map( sunSpotTimer.getRemaining(), 0, SUNSPOT_DURATION/2, 0, 255);
            bri = 255 - map( sunSpotTimer.getRemaining(), 0, SUNSPOT_DURATION/2, 0, 255);            
          }
          setColorOnFace( makeColorHSB(hue, sat, bri), f); //sunspot burst
        }
        else {
          setColorOnFace( makeColorHSB(160, 255, 255), f); // sky color
        }
      }
    }

  }

}
