/* Custom Testing Program for PandOS! Written by Travis Wahl*/
/* Using a randomizer, this program will randomly select a cool fact about sharks
 * then write it to the terminal using WRITETERMINAL, written by Kate Plas & Travis Wahl */
 
#include "h/localLibumps.h"
#include "h/tconst.h"
#include "h/print.h"
 
 
void main() {
 
  int r = rand() % 20; /* Int between 0 and 19 doesnt work yet as there is no lib */
 
 /* Return a random fact based on the number generated! */
  switch(r){
    case 0 ... 5:
      print(WRITETERMINAL, "Sharks do not have bones \n");
      break;
    
    case 6 ... 10:
      print(WRITETERMINAL, "Sharks have electroreceptor organs called ampullae of Lorenzini \n");
      break;
    
    case 11 ... 15:
      print(WRITETERMINAL, "Each Whale Shark's spots are a unique pattern, like a fingerprint \n");
      break;
    
    default:
      print(WRITETERMINAL, "There are over 500 species of sharks \n");
  }
 }
