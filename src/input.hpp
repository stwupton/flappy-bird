#pragma once

struct Input {
	bool flap = false;
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