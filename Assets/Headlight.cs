using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.InputSystem;
using System.IO.Ports;

public class Headlight : MonoBehaviour
{
    System.IO.Ports.SerialPort serialPort;

    private SpriteRenderer spriteRenderer;
    [SerializeField] private Sprite lowSprite;
    [SerializeField] private Sprite highSprite;

    public InputActionReference high;
    public InputActionReference low;

    byte[] buffer = new byte[4];
      
    // Start is called once before the first execution of Update after the MonoBehaviour is created
    void Start()
    {
        spriteRenderer = GetComponent<SpriteRenderer>();

        serialPort = new System.IO.Ports.SerialPort("COM5", 9600);
        serialPort.Open();
    }

    // Update is called once per frame
    void Update()
    {

        if (low.action.ReadValue<float>() > 0.5f)
        {
            if (spriteRenderer != null && lowSprite != null)
            {
                spriteRenderer.sprite = lowSprite;
            }
        }
        else if (high.action.ReadValue<float>() > 0.5f)
        {
            if (spriteRenderer != null && highSprite != null)
            {
                spriteRenderer.sprite = highSprite;
                Debug.Log("HIGH");
            }
        }

        if (serialPort != null && serialPort.IsOpen)
        {
            try
            {
                // Read data from the serial port
                if (serialPort.BytesToRead > 0)
                {
                    int data = serialPort.ReadByte();
                    if (data == 1)
                    {
                        Debug.Log("LOW");
                        if (spriteRenderer != null && lowSprite != null)
                        {
                            spriteRenderer.sprite = lowSprite;
                        }
                    }
                    else if (data == 2)
                    {
                        Debug.Log("HIGH");
                        if (spriteRenderer != null && highSprite != null)
                        {
                            spriteRenderer.sprite = highSprite;
                        }
                    }
                }
            }
            catch (System.Exception e)
            {
                Debug.LogError("Serial port error: " + e.Message);
            }
        }
    }
}
