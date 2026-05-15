using UnityEngine;
#if ENABLE_WINMD_SUPPORT
using System.IO.Ports;
#endif

public class Burger : MonoBehaviour
{
#if ENABLE_WINMD_SUPPORT
    System.IO.Ports.SerialPort serialPort;
#endif

    private SpriteRenderer spriteRenderer;
    [SerializeField] private Sprite lowSprite;
    [SerializeField] private Sprite highSprite;

    // Start is called once before the first execution of Update after the MonoBehaviour is created
    void Start()
    {
        spriteRenderer = GetComponent<SpriteRenderer>();
#if ENABLE_WINMD_SUPPORT
        serialPort = new System.IO.Ports.SerialPort();
        serialPort.PortName = "COM5"; // Set the appropriate COM port
        serialPort.BaudRate = 9600; // Set the baud rate
        serialPort.Parity = System.IO.Ports.Parity.None;
        serialPort.DataBits = 8;
        serialPort.StopBits = System.IO.Ports.StopBits.One;
        serialPort.Handshake = System.IO.Ports.Handshake.None;

        try
        {
            serialPort.Open(); // Open the serial port
        }
        catch (System.Exception e)
        {
            Debug.LogError("Failed to open serial port: " + e.Message);
        }
#else
        Debug.LogError("SerialPort is not available on this platform. Please use Windows Standalone build.");
#endif
    }

    // Update is called once per frame
    void Update()
    {
#if ENABLE_WINMD_SUPPORT
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
#endif
    }
}
