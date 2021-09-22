/*
   Search for any "OFF" Blinks, if none found, signal win condition

   How to use:
    Single Click = turn a Blink on or off (toggle)
    Double Click = start search from this Blink
*/

enum winSearchValues {CHILL, SEARCHING, WAITING, FOUND_OFF, NO_FOUND_OFF, VICTORY, DEFEAT, RESOLVE};

bool isSearchingForWin;
bool isWaitingOnNeighbor;
bool foundWin = false;
byte neighborSearchingForWin;    //changing index
byte indexOfNeighborToReportTo = 6;  // use this for where we started our search

bool isOn = true;
bool flashOn = false;

byte faceValues[6] = {CHILL, CHILL, CHILL, CHILL, CHILL, CHILL};


Timer slowTimer;
#define FRAME_DELAY 0

Timer flashTimer;
#define FLASH_DELAY 100

void setup() {


}

void loop() {

  if (slowTimer.isExpired()) {
    slowTimer.set(FRAME_DELAY);

    if (buttonSingleClicked()) {
      // I am one that was clicked
      isOn = !isOn;

    }
    if (buttonDoubleClicked()) {

      //Start Search for any off blinks
      //If I am an off blink, no need to search
      if (!isOn) {
        isSearchingForWin = false;
      }
      else {
        //Else Ask each neighbor to search for off blinks
        isSearchingForWin = true;
        setAllTo(SEARCHING);
        neighborSearchingForWin = 0;
        indexOfNeighborToReportTo = 6;  //special for master blink
      }

    }

    checkForWin();

  } // end slow timer


  // communicate with neighbors
  FOREACH_FACE(f) {
    setValueSentOnFace(faceValues[f], f);
  }

  // Listen for message from neighbor to search for any off blinks
  // if I am off,
  // return message to neighbor asking me to search; message should say "i am an off blink"
  // else if any of my neighbors are not yet searched blinks
  // then ask them to search for any off blinks


  if (isOn) {
    setColor(WHITE);
  }
  else {
    setColor(dim(BLUE, 64));
  }

  // DEBUG VISUALIZATION
  FOREACH_FACE(f) {
    switch (faceValues[f]) {
      case CHILL:
        {
          if (isOn) {
            setColorOnFace(WHITE, f);
          }
          else {
            setColorOnFace(dim(BLUE, 64), f);
          }
        }
        break;
      case SEARCHING:     setColorOnFace(BLUE, f);    break;
      case WAITING:       setColorOnFace(YELLOW, f);  break;
      case FOUND_OFF:     setColorOnFace(RED, f);     break;
      case NO_FOUND_OFF:  setColorOnFace(GREEN, f);   break;
      case DEFEAT:        setColorOnFace(ORANGE, f);  break;
      case VICTORY:       setColorOnFace(MAGENTA, f); break;
      case RESOLVE:       setColorOnFace(makeColorHSB(random(255), 255, 255), f); break;
    }
  } // end face loop

  if (flashTimer.isExpired()) {
    flashOn = !flashOn;
    flashTimer.set(FLASH_DELAY);

  }
  if (flashOn && isSearchingForWin) {
    setColorOnFace(OFF, neighborSearchingForWin);
  }
  //END DEBUG VISUALIZATION

}

void checkForWin() {
  if (isSearchingForWin) {

    //check if done
    if (isDoneSearching()) {
      isSearchingForWin = false;
      if (indexOfNeighborToReportTo == 6) { // if I am the origin
        foundWin = true;
        setAllTo(VICTORY);
      }
    }

    if (isValueReceivedOnFaceExpired(neighborSearchingForWin)) { // no neighbor!
      faceValues[neighborSearchingForWin] = NO_FOUND_OFF;
      neighborSearchingForWin = (neighborSearchingForWin + 1) % 6;
    }
    else { //found neighbor!
      if (neighborSearchingForWin == indexOfNeighborToReportTo) {
        faceValues[neighborSearchingForWin] = NO_FOUND_OFF;
        faceValues[indexOfNeighborToReportTo] = NO_FOUND_OFF;
        isSearchingForWin = false;
      }
      else {

        byte neighborValue = getLastValueReceivedOnFace(neighborSearchingForWin);

        if (!isWaitingOnNeighbor) { // if neighbor has not yet been asked to search

          if (neighborValue == CHILL) {
            faceValues[neighborSearchingForWin] = WAITING;
            isWaitingOnNeighbor = true;
          }
          else if (neighborValue == SEARCHING ) {
            //ignore and move on to the next one
            faceValues[neighborSearchingForWin] = NO_FOUND_OFF;
            neighborSearchingForWin = (neighborSearchingForWin + 1) % 6;  // continue looking to the next one
          }
          else if (neighborValue == FOUND_OFF) {
            isSearchingForWin = false;
            neighborSearchingForWin = 0;
            faceValues[indexOfNeighborToReportTo] = FOUND_OFF;
          }
          else if (neighborValue == NO_FOUND_OFF) {
            //Ask neighbor to search for off blinks
            faceValues[neighborSearchingForWin] = NO_FOUND_OFF;
            neighborSearchingForWin = (neighborSearchingForWin + 1) % 6;  // continue looking to the next one
          }

        } // end not waiting on neighbor
        else {  // am waiting on neighor

          // let the one I am reporting to know I am waiting on a neighbor
          if (indexOfNeighborToReportTo != 6) { // if I have someone to report to
            faceValues[indexOfNeighborToReportTo] = WAITING;
          }

          if (neighborValue == FOUND_OFF) { // just heard back from neighbor that they FOUND AN OFF
            faceValues[neighborSearchingForWin] = FOUND_OFF;
            isSearchingForWin = false;
            isWaitingOnNeighbor = false;
            neighborSearchingForWin = 0;
            faceValues[indexOfNeighborToReportTo] = FOUND_OFF;
            // if I am the starting point..
            // signal no victory, stop the search, cuz we found an off
            if (indexOfNeighborToReportTo == 6) {
              setAllTo(DEFEAT);
            }
          }
          else if (neighborValue == NO_FOUND_OFF) { // just heard back from neighbor that they haven't found and OFF
            // Acknowledge and move on to the next one
            faceValues[neighborSearchingForWin] = NO_FOUND_OFF;
            isWaitingOnNeighbor = false;
            // if I am the starting point..
            // signal victory, cuz we searched all and didn't find an OFF

            if (indexOfNeighborToReportTo != 6) { // I'm not the starting point
              neighborSearchingForWin = (neighborSearchingForWin + 1) % 6;
            }
            else { // I'm the starting point
              if (neighborSearchingForWin == 5) { // last side I will search
                // signal victory, cuz we searched all and didn't find an OFF
                isSearchingForWin = false;
                foundWin = true;
                setAllTo(VICTORY);
              }
              else {
                // advance to the next side, if we have search all sides, stop searching
                neighborSearchingForWin = (neighborSearchingForWin + 1) % 6;
              }
            }
          }
        } //end waiting on neighbor
      }
    }
  }
  else // not yet searching, listening for the opportunity to participate
  {

    //      if(faceValues[0] == RESOLVE) {
    //        byte value = CHILL;
    //        FOREACH_FACE(f) {
    //        if (!isValueReceivedOnFaceExpired(f)) {
    //          byte neighborValue = getLastValueReceivedOnFace(f);
    //          if(neighborValue == VICTORY) {
    //            value = RESOLVE;
    //          }
    //        }
    //      }

    // look to present neighbors
    if (faceValues[0] == VICTORY) {
      if (buttonPressed()) {
        setAllTo(DEFEAT);
      }
    }

    FOREACH_FACE(f) {
      if (!isValueReceivedOnFaceExpired(f)) {
        byte neighborValue = getLastValueReceivedOnFace(f);


        if (faceValues[0] == DEFEAT) {
          byte value = RESOLVE;

          //listen for neighbors
          FOREACH_FACE(f) {
            if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
              if (getLastValueReceivedOnFace(f) != DEFEAT && getLastValueReceivedOnFace(f) != RESOLVE) {//a neighbor saying
                value = DEFEAT;  // remain in DEFEAT
              }
            }
          }

          setAllTo(value);
        }
        else if (faceValues[0] == RESOLVE) {
          byte value = CHILL;

          //listen for neighbors
          FOREACH_FACE(f) {
            if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
              if (getLastValueReceivedOnFace(f) != RESOLVE && getLastValueReceivedOnFace(f) != CHILL) {//a neighbor saying
                value = RESOLVE;  // remain in VICTORY
              }
            }
          }

          setAllTo(value);
          if (value == CHILL) {
            resetPiece();
          }
        }
        else {
          if (neighborValue == VICTORY) {
            setAllTo(VICTORY);  // spread the victory
          }
          else if (neighborValue == DEFEAT) {
            setAllTo(DEFEAT);  // spread the defeat
          }

          if (neighborValue == WAITING && f != indexOfNeighborToReportTo) { //if neighbor face value is searching

            // set all to searching
            setAllTo(SEARCHING);

            if (!isOn) { //if off
              faceValues[f] = FOUND_OFF;
              setAllTo(FOUND_OFF);
            }
            else { //if on
              // I've been asked to search
              indexOfNeighborToReportTo = f;
              faceValues[f] = WAITING;
              neighborSearchingForWin = (f + 1) % 6; //move clockwise on faces
              isSearchingForWin = true;
            }

          }

        } // end if neighbor is searching

      }// end if neighbor is present


    } // end for loop
  } // end not yet searching

  buttonPressed();
}

void resetPiece() {
  setAllTo(CHILL);
  isSearchingForWin = false;
  isWaitingOnNeighbor = false;
  foundWin = false;
  neighborSearchingForWin = 0;
  indexOfNeighborToReportTo = 6;
}

void setAllTo(byte state) {
  FOREACH_FACE(f) {
    faceValues[f] = state;
  }
}

bool isDoneSearching() {
  FOREACH_FACE(f) {
    if (faceValues[f] == CHILL || faceValues[f] == WAITING || faceValues[f] == SEARCHING) {
      return false;
    }
  }
  return true;
}
