package me.devilsen.czxing.thread;

import java.util.Deque;
import java.util.concurrent.BlockingDeque;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.LinkedBlockingDeque;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

import me.devilsen.czxing.util.BarCodeUtil;

/**
 * desc : 任务分发器
 * date : 2019-06-29 11:32
 *
 * @author : dongSen
 */
//TODO: 场景扫码配置
public final class Dispatcher {

    private static final String TAG = Dispatcher.class.getSimpleName();

    private static final int MAX_RUNNABLE = 10;
    private ExecutorService executorService;

    private final BlockingDeque<Runnable> blockingDeque;
    private int corePoolSize = 2;
    private int maximumPoolSize = 4;

    public Dispatcher() {
        blockingDeque = new LinkedBlockingDeque<>();
    }

    public ProcessRunnable newRunnable(FrameData frameData, Callback callback) {
        return new ProcessRunnable(this, frameData, callback);
    }

    public ProcessRunnable newRunnable(byte[] data, int left, int top, int width, int height, int rowWidth, int rowHeight, Callback callback) {
        return newRunnable(new FrameData(data, left, top, width, height, rowWidth, rowHeight), callback);
    }

    synchronized int enqueue(ProcessRunnable runnable) {
        if (blockingDeque.size() > MAX_RUNNABLE) {
            blockingDeque.remove();
        }

        execute(runnable);
        BarCodeUtil.d("blockingDeque: " + blockingDeque.size());
        return blockingDeque.size();
    }

    private synchronized void execute(Runnable runnable) {
        getService().execute(runnable);
    }

    public void finished(ProcessRunnable runnable) {
        finish(blockingDeque, runnable);
    }

    private void finish(Deque<Runnable> decodeDeque, ProcessRunnable runnable) {
        synchronized (this) {
            if (decodeDeque.size() > 0) {
                decodeDeque.remove(runnable);
                promoteCalls();
            }
        }
    }

    private ExecutorService getService(){
        if (executorService == null){
            executorService = new ThreadPoolExecutor(corePoolSize,
                    maximumPoolSize,
                    10,
                    TimeUnit.SECONDS,
                    blockingDeque,
                    ExecutorUtil.threadFactory("decode dispatcher", false));
        }
        return executorService;
    }

    private synchronized void promoteCalls() {
        if (blockingDeque.isEmpty()) {
            return;
        }

        Runnable first = blockingDeque.getFirst();
        execute(first);
    }

    public synchronized void cancelAll() {
        for (Runnable runnable : blockingDeque) {
            ((ProcessRunnable) runnable).cancel();
        }
        blockingDeque.clear();
    }

    public int getCorePoolSize() {
        return corePoolSize;
    }

    public void setCorePoolSize(int corePoolSize) {
        this.corePoolSize = corePoolSize;
    }

    public int getMaximumPoolSize() {
        return maximumPoolSize;
    }

    public void setMaximumPoolSize(int maximumPoolSize) {
        this.maximumPoolSize = maximumPoolSize;
    }

}
