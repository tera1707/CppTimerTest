#include <windows.h>
#include <stdio.h>

// SetWaitableTimerの解説
// https://www.codeproject.com/articles/49798/wake-the-pc-from-standby-or-hibernation?fid=1556013&df=90&mpp=25&prof=True&sort=Position&view=Normal&spc=Relaxed&fr=26
// SetWaitableTimerのfResumeフラグをONにすると、スリープ中でもカウントしてくれそう。だが、ハードやwindowsの設定、その他諸々の条件が揃うと、タイマ満了時に
// resumeしてしまいそう。
// →ディスプレイONジイベントで、24h経過を見る方がよさそう。

HANDLE hTimer = NULL;
LARGE_INTEGER liDueTime;
long long sec = 5LL;//タイマ満了時間(秒)

/// <param name="lpArg">SetWaitableTimerの第四引数で渡すパラメータ</param>
/// <param name="dwTimerLowValue">時刻が載ってるらしい（使う気なし）</param>
/// <param name="dwTimerHighValue">時刻が載ってるらしい（使う気なし）</param>
VOID CALLBACK TimerAPCProc(LPVOID lpArg, DWORD dwTimerLowValue, DWORD dwTimerHighValue)
{
    auto param = (int*)lpArg;
    printf("タイマ満了来ました。%d\r\n", *param);
}

// 満了時コールバック無し、シグナル待ちするパターン
// fResume:SetWaitableTimerのタイマ満了時に、スリープをresumeさせるかどうか
// →これをTRUEにしていると、スリープ/休止中もカウントを継続できる。で、スリープ中に満了したら、ハードやwindowsの設定が許せば、resumeするらしい。(手持ちPCでは起動しなかった)）
void SetWaitableTimerWithoutCallback(HANDLE hTimer, LARGE_INTEGER liDueTime, BOOL fResume)
{
    MessageBox(NULL, L"start", L"", MB_OK);
    if (!SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, fResume))
    {
        printf("SetWaitableTimer 失敗 (%d)\r\n", GetLastError());
        return;
    }
    printf("SetWaitableTimer 成功 (%d)\r\n", GetLastError());

    // タイマー満了待ち
    if (WaitForSingleObject(hTimer, INFINITE) != WAIT_OBJECT_0)
    {
        printf("WaitForSingleObject 失敗 (%d)\r\n", GetLastError());
    }
    else
    {
        printf("タイマーがシグナル状態になりました\r\n");
    }
    MessageBox(NULL, L"stop", L"", MB_OK);
}

void SetWaitableTimerWithCallback(HANDLE hTimer, LARGE_INTEGER liDueTime, BOOL fResume)
{
    MessageBox(NULL, L"start", L"", MB_OK);

    // 満了時コールバック有りパターン
    int param = 99;
    if (!SetWaitableTimer(hTimer, &liDueTime, 0, TimerAPCProc, &param, fResume))
    {
        printf("SetWaitableTimer 失敗 (%d)\r\n", GetLastError());
        return;
    }
    // 注意！Sleep()で待っていると、ここと同じスレッドで実行されることになるTimerAPCProcが呼べなくなる。
    // WaitableTimerを使っている場合にスレッドをスリープさせたい場合は、SleepEx()で行う必要がある。
    // (SleepEx()の第三引数はTRUE=アラート可能である必要がある。)
    // また、タイマ満了した結果、SleepEx()の待ちは終了する
    SleepEx(INFINITE, TRUE);

    MessageBox(NULL, L"stop", L"", MB_OK);
}

int main()
{
    auto oneSec = -10000000LL;// 1=100nsec(ナノ秒)なので、これで1秒になる
    liDueTime.QuadPart = sec * oneSec;

    // タイマ作成
    hTimer = CreateWaitableTimer(NULL, TRUE, L"aaa");
    if (NULL == hTimer)
    {
        printf("CreateWaitableTimer 失敗 (%d)\r\n", GetLastError());
        return 1;
    }
#if 0
    SetWaitableTimerWithoutCallback(hTimer, liDueTime, FALSE);
#else
    SetWaitableTimerWithCallback(hTimer, liDueTime, FALSE);
#endif

    return 0;
}
