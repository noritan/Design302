/* ========================================
 *
 * Copyright noritan.org, 2016
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF NORITAN.ORG.
 *
 * ========================================
*/
#include "project.h"

#define     IN_EP               (0x02u)
#define     OUT_EP              (0x01u)
#define     BUFFER_SIZE         (64u)

uint8       buffer_in[BUFFER_SIZE] = "@@ABCDEFGIHJKLMNOPQRSTUVWXYZ";
uint8       buffer_out[BUFFER_SIZE];
uint16      length;


int main(void) {
    CyGlobalIntEnable;                  // 割り込みの有効化    
    USB_Start(0, USB_5V_OPERATION);     // 動作電圧5VにてUSBFSコンポーネントを初期化

    for (;;) {
        // 初期化終了まで待機
        while (USB_GetConfiguration() == 0);

        USB_IsConfigurationChanged();   // CHANGEフラグを確実にクリアする

        USB_EnableOutEP(OUT_EP);        // OUTエンドポイントでホストからデータを受信する

        for (;;) {
            // 設定が変更されたら、再初期化をおこなう
            if (USB_IsConfigurationChanged()) {
                break;
            }

            // BULK-OUT : データが受信されたか調べる
            if (USB_GetEPState(OUT_EP) == USB_OUT_BUFFER_FULL) {
                // 受信バイト数を取得する
                length = USB_GetEPCount(OUT_EP);

                // OUTバッファからデータを取り出す
                USB_ReadOutEP(OUT_EP, &buffer_out[0], length);
            }
            
            // BULK-IN : 送信バッファが空いたか調べる
            if (USB_GetEPState(IN_EP) == USB_IN_BUFFER_EMPTY) {
                // INバッファにデータを送り込む
                buffer_in[0]++;
                USB_LoadInEP(IN_EP, &buffer_in[0], BUFFER_SIZE);
            }
        }
    }
}

/* [] END OF FILE */
