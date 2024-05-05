using UnityEngine;
using Uduino;
using System;
using Kandooz;
using TMPro;

public class GloveController : MonoBehaviour
{
    public GameObject obj;
    public TMP_Text text;
    public Kandooz.KVR.HandAnimationController controller;

    private float[] values = new float[14];
    private string msg = "";
    private bool isCalibrate = true;
    private float t = 0f;

    private float index_min = 9999f;
    private float index_max = -9999f;
    private float middle_min = 9999f;
    private float middle_max = -9999f;
    private float ring_min = 9999f;
    private float ring_max = -9999f;
    private float pinky_min = 9999f;
    private float pinky_max = -9999f;
    private float thumb_min = 9999f;
    private float thumb_max = -9999f;

    private float angleX_avg = 0;
    private float angleY_avg = 0;
    private float angleZ_avg = 0;
    private int n = 0;

    private float angleX0 = 7.6f;
    private float angleY0 = 180f;
    private float angleZ0 = 0f;

    private void Start()
    {
        t = Time.time;
    }

    // Update is called once per frame
    void Update()
    {
        values = Array.ConvertAll(msg.Split(','), float.Parse);

        float index = values[3] + values[4];
        float middle = values[5] + values[6];
        float ring = values[7] + values[8];
        float pinky = values[9] + values[10];
        float thumb = values[11] + values[12];

        float dt = Time.time - t;

        if (isCalibrate)
        {
            if (dt < 5f)
            {
                text.text = "Beginning Calibration. Please make a fist and hold that pose.";
            }
            else if (dt >= 5f && dt < 15f)
            {
                text.text = "Now Calibrating...";

                if (index < index_min) index_min = index;
                if (middle < middle_min) middle_min = middle;
                if (ring < ring_min) ring_min = ring;
                if (pinky < pinky_min) pinky_min = pinky;
                if (thumb < thumb_min) thumb_min = thumb;
            }
            else if (dt >= 15f && dt < 20f)
            {
                text.text = "Now, please relax your hand and place it on a flat surface.";
            }
            else if (dt >= 20f && dt < 30f)
            {
                text.text = "Resuming Calibration...";
                if (index > index_max) index_max = index;
                if (middle > middle_max) middle_max = middle;
                if (ring > ring_max) ring_max = ring;
                if (pinky > pinky_max) pinky_max = pinky;
                if (thumb > thumb_max) thumb_max = thumb;

                angleX_avg += values[1];
                angleY_avg += values[2];
                angleZ_avg += values[0];
                n++;
            }
            else if (dt >= 30f)
            {
                text.text = "Calibration complete.";

                angleX_avg /= n;
                angleY_avg /= n;
                angleZ_avg /= n;

                isCalibrate = false;
            }
        }
        else
        {
            if (dt >= 33f)
            {
                text.text = "";
            }
            //  0 -> Angle X
            //  1 -> Angle Y
            //  2 -> Angle Z
            //  3 -> Index Knuckle
            //  4 -> Index Finger
            //  5 -> Middle Knuckle
            //  6 -> Middle Finger
            //  7 -> Ring Knuckle
            //  8 -> Ring Finger
            //  9 -> Pinky Knuckle
            // 10 -> Pinky Finger
            // 11 -> Thumb Knuckle
            // 12 -> Thumb Finger
            // 13 -> Thumb Adductor

            obj.transform.rotation = Quaternion.Euler(new Vector3(-values[1] - angleX_avg + angleX0, values[2] - angleX0 + angleY0, values[0] - angleZ_avg + angleZ0));

            controller[1] = Mathf.InverseLerp(index_max, index_min, index);
            controller[2] = Mathf.InverseLerp(middle_max, middle_min, middle);
            controller[3] = Mathf.InverseLerp(ring_max, ring_min, ring);
            controller[4] = Mathf.InverseLerp(pinky_max, pinky_min, pinky);
            controller[0] = Mathf.InverseLerp(thumb_max, thumb_min, thumb);
        }
    }

    public void Received(string data, UduinoDevice u)
    {
        msg = data;
        print(msg);
    }
}
