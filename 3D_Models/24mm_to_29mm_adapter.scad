// Parameters
$fa = 1; // Sets the accuracy of curved objects
$fs = 0.2; // Sets the minimum length of edges for rendering

// Dimensions
outer_diameter = 29 + 1.45; // Outer diameter of the cylinder
inner_diameter = 24 + .5; // Inner diameter of the cylinder
height_29 = 114; // Height of the cylinder
height_24 = 97; // Height of the cylinder

// Create the cylinder
difference() {
  cylinder(d = outer_diameter, h = height_29, center = true); // Create the main cylinder
  
  translate([0, 0, height_29 - height_24]) {
    cylinder(d = inner_diameter, h = height_24, center = true); // Create the hollow part of the cylinder
  }
  
  translate([0, 0, -height_29/2]) {
    cylinder(d = inner_diameter, h = 30, center = true); // Create the hollow part of the cylinder
  }
  

    cylinder(d = inner_diameter-7.5, h = 1000, center = true); // Create the hollow part of the cylinder  

}
