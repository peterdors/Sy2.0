#include "FaceDetect.hpp"
#include <chrono>

FaceDetect::FaceDetect()
{
    Load();
}

void FaceDetect::Load()
{
    // Load the cascades.
    if (!face_cascade.load(kFaceCascadeFile))
    {
        cout << "--(!)Error loading face cascade\n";
        return;
    }

    // Open default camera and read the video stream.
    capture.open(0);

    if (!capture.isOpened())
    {
        cout << "--(!)Error opening video capture\n";
        return;
    }

    capture.set(CAP_PROP_FRAME_WIDTH, kFrameWidth);
    capture.set(CAP_PROP_FRAME_HEIGHT, kFrameHeight);
}

void FaceDetect::Read()
{
    // Load();

    while (true)
    {
        bool face_found = false;

        for (int i = 0; i < 5; i++)
        {
            capture.read(frame);
        }

        if (frame.empty())
        {
            cout << "--(!) No captured frame -- Break!\n";
            break;
        }

        Detect(ref(face_found));

        // if (waitKey(1) == 'q')
        // {
        //     break;
        // }

        // imshow("Capture - Face detection", frame);
    }

    capture.release();
    // destroyAllWindows();
}

void FaceDetect::Detect(bool& face_found)
{
    Mat frame_gray;

    cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
    equalizeHist(frame_gray, frame_gray);

    //-- Detect faces
    std::vector<Rect> faces;
    face_cascade.detectMultiScale(frame_gray, faces);

    if (faces.size() < 1)
    {
        // Fail silently and retry next go around.
        return;
    }

    face_found = true;
    fface = faces[0];
    CenterPoint* curr_center = nullptr, *new_center = nullptr;

    if (face_found)
    {
        curr_center = center.load();
        new_center = new CenterPoint(fface.x + fface.width/2,
            fface.y + fface.height/2);

        while (!center.compare_exchange_weak(curr_center, new_center,
            memory_order_release, memory_order_relaxed))
        {
            curr_center = center.load();
            new_center = new CenterPoint(fface.x + fface.width/2,
                fface.y + fface.height/2);
        }

        Point ctr(fface.x + fface.width/2, fface.y + fface.height/2);
        ellipse( frame, ctr, Size( fface.width/2, fface.height/2 ), 0, 0, 360, Scalar( 0, 255, 0 ), 4 );

    }
    else
    {
        curr_center = center.load();
        new_center = new CenterPoint(kFrameWidth / 2, kFrameHeight / 2);

        while (!center.compare_exchange_weak(curr_center, new_center,
            memory_order_release, memory_order_relaxed))
        {
            curr_center = center.load();
            new_center = new CenterPoint(kFrameWidth / 2, kFrameHeight / 2);
        }
    }
}

void FaceDetect::MoveServos()
{
    const int offset_pan = 100, offset_tilt = 100;
    const int pan_motor_id = 7;
    const int tilt_motor_id = 11;
    const int pan_adj = 10, tilt_adj = 10;

    const int left = (kFrameWidth / 2) - offset_pan;
    const int right = (kFrameWidth / 2) + offset_pan;
    const int top = (kFrameHeight / 2) + offset_tilt;
    const int bottom = (kFrameHeight / 2) - offset_tilt;
    const int adj_offset = 10;

    while (true)
    {
        bool did_tilt = false;
        bool did_pan = false;

        CenterPoint* c = center.load();

        if (center.load()->x == 0 && c->y == 0)
        {
            continue;
        }

        if (center.load()->x < (left - 60))
        {
            did_pan = true;
            sb.set_angle(pan_motor_id, pan_adj + adj_offset);
        }

        else if (center.load()->x < (left))
        {
            did_pan = true;
            sb.set_angle(pan_motor_id, pan_adj);
        }

        else if (center.load()->x > (right + 60))
        {
            did_pan = true;
            sb.set_angle(pan_motor_id, (pan_adj + adj_offset) * -1);
        }

        else if (center.load()->x > (right))
        {
            did_pan = true;
            sb.set_angle(pan_motor_id, (pan_adj) * -1);
        }

        if (did_pan)
        {
            this_thread::sleep_for(chrono::microseconds(500));
        }

        if (center.load()->y > (top + 60))
        {
            did_tilt = true;
            sb.set_angle(tilt_motor_id, tilt_adj + adj_offset);
        }

        else if (center.load()->y > (top))
        {
            did_tilt = true;
            sb.set_angle(tilt_motor_id, tilt_adj);
        }

        else if (center.load()->y < (bottom - 60))
        {
            did_tilt = true;
            sb.set_angle(tilt_motor_id, (tilt_adj + adj_offset) * -1);
        }

        else if (center.load()->y < (bottom))
        {
            did_tilt = true;
            sb.set_angle(tilt_motor_id, (tilt_adj) * -1);
        }

        if (did_tilt)
        {
            this_thread::sleep_for(chrono::microseconds(500));
        }
    }
}

Mat FaceDetect::getFrame()
{
    return frame;
}

void doRead(FaceDetect& fd)
{
    fd.Read();
}

void doMoveServos(FaceDetect& fd)
{
    fd.MoveServos();
}

int main(void)
{
    vector<thread *> threads;

    FaceDetect fd;

    threads.push_back(new thread(doRead, ref(fd)));
    threads.push_back(new thread(doMoveServos, ref(fd)));

    for (auto& t : threads)
    {
        t->join();
    }

    // imshow("Capture - Face detection", fd.getFrame());

    return 0;
}
