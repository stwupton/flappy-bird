#pragma once

struct Input {
	bool flap = false;

	// TODO(steven): Don't know what to call this, it's for modifying the gravity 
	// when the button is held down.
	bool hovering = false;

	void flap_handled() {
		this->flap = false;
	}

	void input_down() {
		this->flap = true;
		this->hovering = true;
	}

	void input_up() {
		this->hovering = false;
	}
};