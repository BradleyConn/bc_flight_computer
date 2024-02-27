// Parameters
cube_size = 5;
cube_height = 7.5;
slot_width = 1.6;


// Slot cutout
difference() {
  cube([cube_size, cube_size, cube_height]); // Create a cube
  translate([(cube_size-slot_width)/2, (cube_size-slot_width)/2, 0]) {// Position the slot in the center of the cube
  cube([slot_width, cube_size, cube_height]); // Cut out the slot
  }
}
