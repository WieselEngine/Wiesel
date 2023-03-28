//   Copyright 2023 Metehan Gezer
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "w_pch.h"

namespace Wiesel {
	struct KeyData {
		KeyData();
		explicit KeyData(bool pressed);

		bool Pressed;
	};

	class KeyManager {
	public:
		KeyManager();
		~KeyManager();

		void Set(int key, bool pressed);
		bool IsPressed(int key);
	private:
		std::map<int, KeyData> m_Keys;
	};
}
