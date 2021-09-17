/*
   Single piece becomes the sun

   rays shoot out of 3 sides and rotate
   rest of the pieces carry the sun's rays
*/

bool isSun = false;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

  if (buttonSingleClicked()) {
    isSun = !isSun; // toggle a sun
  }

  if (isSun) {
    
    bool shineOdd = (byte)(millis() / 1000) % 2;
    
    FOREACH_FACE(f) {
      
      bool isOn;
      
      if (shineOdd) {
        isOn = f % 2;
      }
      else {
        isOn = ((f+1 )% 2);
      }
      
      setValueSentOnFace(isOn, f);
      
      if (isOn) {
        setColorOnFace(YELLOW, f);
      }
      else {
        setColorOnFace(BLUE, f);
      }
      
    }
  }
  else {
    // pass rays from the sun
    setColor(BLUE);
    setValueSentOnAllFaces(0);    
    
    FOREACH_FACE(f) {
      
      if (!isValueReceivedOnFaceExpired(f)) {
        byte neighborVal = getLastValueReceivedOnFace(f);
        if (neighborVal == 1) {
          // this is a sun ray, light up on this face and pass to the opposite face
          setValueSentOnFace(1, (f + 3) % 6);
          setColorOnFace(YELLOW, f);
          setColorOnFace(YELLOW, (f + 3) % 6);
        }
      }
    }
  }
}
