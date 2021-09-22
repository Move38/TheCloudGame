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
byte setupFadeFace;
Timer setupFadeTimer;
word backgroundTime;
#define SETUP_FADE_UP_INTERVAL 1000
#define SETUP_RED_INTERVAL 500
#define SETUP_FADE_DELAY 4000


//Timers

Timer fadeToBright;
Timer sunSpotFade;
Timer fadeToCloud;
Timer fadeToDark;

#define FADE_TO_BRIGHT_DELAY 2000
#define SUN_SPOT_FADE 1000
#define FADE_TO_CLOUD_DELAY 3000
#define FADE_TO_DARK_DELAY 1000


/*
   WIN CONDITION ANIMATION
*/


void setup() {
  // nothing to do here
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
    //    setColor( MAGENTA );
    fadeToNoLight();
    if (fadeToDark.isExpired()) {
      silverLiningDisplay();
    }
  }
  else {
    initWin();
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
   WIN ANIMATION
*/
void initWin() {
  //Set Timers
  setupFadeTimer.set(backgroundTime + SETUP_FADE_UP_INTERVAL + random(SETUP_FADE_DELAY));
  fadeToBright.set(FADE_TO_BRIGHT_DELAY);
  fadeToDark.set(FADE_TO_DARK_DELAY);
  fadeToCloud.set(FADE_TO_CLOUD_DELAY);
}

void fadeToNoLight() {
  FOREACH_FACE(f) {
    byte brightness = map(fadeToDark.getRemaining(), 0, FADE_TO_DARK_DELAY, 0, 255);
    Color faceColor = makeColorHSB(0, 0, brightness);
    setColorOnFace(faceColor, f);
  }
}

void silverLiningDisplay() {
  Color faceColor_lining;
  Color faceColor_cloud;

  FOREACH_FACE(f) {
    // minimum of 125, maximum of 255
    byte phaseShift = 60 * f;
    byte amplitude = 55;
    byte midline = 185;
    byte rate = 10;
    byte brightness = 255 -  map(fadeToBright.getRemaining(), 0, FADE_TO_BRIGHT_DELAY - FADE_TO_DARK_DELAY, 0, 255);
    byte cloudBrightness = 255 -  map(fadeToCloud.getRemaining(), 0, FADE_TO_CLOUD_DELAY - FADE_TO_DARK_DELAY, 0, 255);
    if (!fadeToCloud.isExpired()) {

      faceColor_lining = makeColorHSB(0, 0, brightness);
      faceColor_cloud = makeColorHSB(160, cloudBrightness, cloudBrightness);

    }
    else {
      faceColor_lining = makeColorHSB(0, 0, 255);
      faceColor_cloud = makeColorHSB(160, 255, 255);

    }
    silverLining(faceColor_cloud, faceColor_lining);
  }
}




void silverLining(Color faceColor_Cloud, Color faceColor_Lining) {

  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) { //if something there aka if within the cloud

      Color fadeColor;
      byte saturation;
      byte hue;
      // have the color on the Blink raise and lower to feel more alive
      byte bri = 185 + sin8_C( (millis() / 14) % 255) * 70 / 255; // oscillate between values 185and 255


      if (setupFadeTimer.isExpired()) { //fade timer for sun spots
        setupFadeFace = f; //assign face
        backgroundTime = SETUP_RED_INTERVAL + random(SETUP_RED_INTERVAL / 2); //bit of randomness
        setupFadeTimer.set(backgroundTime + SETUP_FADE_UP_INTERVAL + random(SETUP_FADE_DELAY));
      }


      if (setupFadeTimer.getRemaining() < backgroundTime + SETUP_FADE_UP_INTERVAL) {//we are inside the animation
        if (setupFadeTimer.getRemaining() < SETUP_FADE_UP_INTERVAL) {//we are fading
          saturation = 255 - map(setupFadeTimer.getRemaining(), 0, SETUP_FADE_UP_INTERVAL, 0, 255);
          fadeColor = makeColorHSB(160, saturation, bri); //sun spot fade
        } else {
          sunSpotFade.set(SUN_SPOT_FADE);
          if (!sunSpotFade.isExpired()) {
            saturation =  map(sunSpotFade.getRemaining(), SUN_SPOT_FADE, 0, 0, 255);
            fadeColor = makeColorHSB(sunSpot_hue, saturation, bri); //sunspot burst
          }

        }

        setColorOnFace(fadeColor, setupFadeFace); //set sun spot colors
      }

      //fadeColor = makeColorHSB(160, 255, bri); //cloud colour
      setColorOnFace(faceColor_Cloud, f);  //set cloud colours
    }


    else {
      setColorOnFace(faceColor_Lining, f); // if not within the cloud, display the lining color (edges)

    }

  }
}
