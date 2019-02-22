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
uint16      length_out;

#define     ST_ACKWAIT          (1u)
#define     ST_READING          (2u)
#define     ST_GETWAIT          (3u)
#define     ST_DISCARDING       (4u)
#define     ST_PREPARING        (5u)

uint8       state_in;           // State code for BULK-IN
uint8       state_out;          // State code for BULK-OUT

int main(void) {
    CyGlobalIntEnable;                  // 割り込みの有効化    
    USB_Start(0, USB_5V_OPERATION);     // 動作電圧5VにてUSBFSコンポーネントを初期化

    for (;;) {
        // 初期化終了まで待機
        while (USB_GetConfiguration() == 0);

        USB_IsConfigurationChanged();   // CHANGEフラグを確実にクリアする

        // BULK-OUT: OUTエンドポイントでホストからデータを受信する
        state_out = ST_DISCARDING;

        // BULK-IN: 初期状態を決定する
        state_in = ST_PREPARING;
        
        for (;;) {
            // 設定が変更されたら、再初期化をおこなう
            if (USB_IsConfigurationChanged()) {
                break;
            }

            // BULK-OUT ステートマシン
            switch (state_out) {
                case ST_DISCARDING:
                    // OUTバッファのデータを破棄する
                    USB_EnableOutEP(OUT_EP);
                    state_out = ST_ACKWAIT;
                    break;
                case ST_ACKWAIT:
                    // ホストからのパケットの到着を待つ
                    if (USB_GetEPState(OUT_EP) == USB_OUT_BUFFER_FULL) {
                        state_out = ST_READING;
                    }
                    break;
                case ST_READING:
                    // 受信バイト数を取得する
                    length_out = USB_GetEPCount(OUT_EP);
                    // OUTバッファからデータを取り出す
                    USB_ReadOutEP(OUT_EP, &buffer_out[0], length_out);
                    state_out = ST_GETWAIT;
                    break;
                case ST_GETWAIT:
                    // OUTバッファからの転送を待つ
                    if (USB_GetEPState(OUT_EP) != USB_OUT_BUFFER_FULL) {
                        state_out = ST_DISCARDING;
                    }
                    break;
                default:
                    break;
            }
            
            // BULK-IN ステートマシン
            switch (state_in) {
                case ST_PREPARING:
                    // 空きバッファにデータを準備する
                    buffer_in[0]++;
                    // INバッファのデータを送信する
                    USB_LoadInEP(IN_EP, &buffer_in[0], BUFFER_SIZE);
                    state_in = ST_ACKWAIT;
                    break;
                case ST_ACKWAIT:
                    // ホストからのデータ受信確認を待つ
                    if (USB_GetEPState(IN_EP) == USB_IN_BUFFER_EMPTY) {
                        state_in = ST_PREPARING;
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

/* [] END OF FILE */
