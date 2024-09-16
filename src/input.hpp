#pragma once

struct Input {
	bool flap = false;

	// TODO(steven): Don't know what to call this, it's for modifying the gravity 
	// when the button is held down.
	bool hovering = false;
};