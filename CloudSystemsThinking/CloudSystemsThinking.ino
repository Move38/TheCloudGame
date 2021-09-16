/*
   Celebrate win if there are not any "OFF" Blinks
   The systems thinking approach

   How to use:
    Single Click = turn a Blink on or off (toggle)
*/

bool isOn = false;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

  if (buttonSingleClicked()) {
    // I am one that was clicked
    isOn = !isOn;
  }

  // propogate inverse of distance from OFF blink
  byte inverseDistanceFromOff = 0;

  // if it is not on, then we are the OFF blink, and the inverse distance is 63
  if(!isOn) {
    inverseDistanceFromOff = 63;
  }

  // now search neighbors for closest to OFF blink
  FOREACH_FACE(f) {
    if(!isValueReceivedOnFaceExpired(f)) {
      byte neighborVal = getLastValueReceivedOnFace(f);
      if( neighborVal > inverseDistanceFromOff ) {
        inverseDistanceFromOff = neighborVal-1;
      }
    }
  }
  
  setValueSentOnAllFaces(inverseDistanceFromOff);

  // display our state
  if(isOn) {
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

    if(inverseDistanceFromOff == 0) {
    // Win is true
    setColorOnFace(MAGENTA, 1);
    setColorOnFace(MAGENTA, 3);
    setColorOnFace(MAGENTA, 5);
  }
}
