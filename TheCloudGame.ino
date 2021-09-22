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

void setup() {
  // nothing to do here
}


void loop() {

  //  if (slowTimer.isExpired()) {
  //    slowTimer.set(DELAY);
  /*
     INPUTS
  */
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
      else if ( neighborVal == RESET ) {
        if ( resetTimer.isExpired() ) {
          resetTimer.set( RESET_DURATION );
          isOn = false;
          break;  // get out of this for loop
        }
      }

      if ( neighborVal <= MAX_DISTANCE && neighborVal > inverseDistanceFromOff ) {
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
    setColorOnFace(dim(BLUE, 128), 0);
    setColorOnFace(dim(BLUE,  64), 1);
    setColorOnFace(dim(BLUE, 128), 2);
    setColorOnFace(dim(BLUE,  64), 3);
    setColorOnFace(dim(BLUE, 128), 4);
    setColorOnFace(dim(BLUE,  64), 5);
  }

  if ( myCommState == 0 ) {
    // display win
    setColor( MAGENTA );
  }

  if ( !resetTimer.isExpired() ) {
    // display reset
    setColor( dim( WHITE, map(resetTimer.getRemaining(), 0, RESET_DURATION, 0, 255) ) );
  }

  /*
     DEBUG DISPLAY
  */
  switch ( myCommState ) {
    case FLIP:  setColorOnFace( BLUE,  0 );   break;
    case ACK:   setColorOnFace( GREEN, 0 );   break;
    case RESET: setColorOnFace( RED,   0 );   break;
    case 0:     setColorOnFace( MAGENTA, 0);  break;
    default:    setColorOnFace( YELLOW, 0);   break;
  }
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
