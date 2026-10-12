#pragma once

namespace ClassLink_Win
{

// Retail class references, called from WinMain
void ReferenceGameClasses();
void ReferenceEngineClasses();

// Calls the retail ones and references the other classes the linker would strip
void ReferenceClasses();

} // namespace ClassLink_Win
