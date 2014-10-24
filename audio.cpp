// Audio functions

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QByteArray>
#include <QBuffer>
#include <QFile>
#include <QAudioBuffer>
#include <QAudioOutput>

#include "math.h"

// Generate 8-bit PCM waveform (simple sine wave)
QByteArray MainWindow::pcmWaveform( int rateHz, int durationMs, int freqHz, int amplitudeStart, int amplitudeEnd )
{
    int sampleCount = rateHz * durationMs / 1000;
    QByteArray r(sampleCount, 0);
    double spc = ((double)rateHz) / freqHz; // Samples per cycle
    double increment = 2 * M_PI / spc; // Increment in radans for each cycle
    int n;
    double x = 0.0;
    for (n = 0; n < sampleCount; n++)
    {
        int amplitude = amplitudeStart + n * (amplitudeEnd - amplitudeStart) / sampleCount;
        r[n] = amplitude * sin(x);
        x += increment;
    }
    return r;
}

void MainWindow::handleStateChanged(QAudio::State newState)
{
    switch (newState) {
        case QAudio::IdleState:
            // Finished playing (no more data)
            m_audio->stop();
            m_audioData.close();
            //delete audio;
            break;

        case QAudio::StoppedState:
            // Stopped for other reasons
            if (m_audio->error() != QAudio::NoError) {
                // Error handling
                qDebug() << "Error occurred in audio playback";
            }
            break;

        default:
            // ... other cases as appropriate
            break;
    }
}

void MainWindow::startPlay( QByteArray& pcm )
{
    m_audioData.setBuffer(&pcm);
    m_audioData.open(QBuffer::ReadOnly);
    m_audioData.seek(0);
    connect( m_audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)) );
    m_audio->start(&m_audioData);
}

void MainWindow::testLinkHandler(QString cmd)
{
    qDebug() << "Test:" << cmd;
    startPlay(m_pcmBeep2);
}
