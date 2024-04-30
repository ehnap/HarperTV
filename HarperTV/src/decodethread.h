#include <QThread>

class DecodeThread : public QThread {
public:
    DecodeThread(const std::string& url, 
        QMutex* mutex,
        QWaitCondition* frameAvailable,
        QQueue<AVFrame*>* frameQueue);
    void run() override;

private:
    std::string m_url;
    QMutex* m_mutex;
    QWaitCondition* m_frameAvailable;
    QQueue<AVFrame*>* m_frameQueue;
};