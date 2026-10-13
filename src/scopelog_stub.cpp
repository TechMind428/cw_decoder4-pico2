// Pico 2版はシリアル・スコープログ機能を組み込まない。
// v3.2デコーダーが要素タイミングを通知するための互換スタブ。
#include <stdint.h>

void scopelog_element(uint8_t, uint32_t, uint32_t)
{
}
