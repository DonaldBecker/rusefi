package com.rusefi;

import com.rusefi.autodetect.PortDetector;
import com.rusefi.io.LinkManager;
import com.rusefi.io.serial.PortHolder;
import com.rusefi.maintenance.*;
import com.rusefi.ui.util.HorizontalLine;
import com.rusefi.ui.util.URLLabel;
import com.rusefi.ui.util.UiUtils;
import net.miginfocom.swing.MigLayout;
import org.jetbrains.annotations.NotNull;
import org.putgemin.VerticalFlowLayout;

import javax.swing.*;
import javax.swing.border.TitledBorder;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import static com.rusefi.ui.storage.PersistentConfiguration.getConfig;
import static com.rusefi.ui.util.UiUtils.setToolTip;

/**
 * This frame is used on startup to select the port we would be using
 *
 * @author Andrey Belomutskiy
 * <p/>
 * 2/14/14
 * @see SimulatorHelper
 * @see FirmwareFlasher
 */
public class StartupFrame {
    // todo: figure out a better way to work with absolute path
    private static final String APPICON = "appicon.png";
    private static final String LOGO = "logo.gif";
    public static final String LINK_TEXT = "rusEfi (c) 2012-2019";
    private static final String URI = "http://rusefi.com/?java_console";
    // private static final int RUSEFI_ORANGE = 0xff7d03;

    private final JFrame frame;
    private final JPanel connectPanel = new JPanel(new FlowLayout());
    // todo: move this line to the connectPanel
    private final JComboBox<String> comboPorts = new JComboBox<>();
    @NotNull
    private List<String> currentlyDisplayedPorts = new ArrayList<>();
    private boolean isFirstTimeApplyingPorts = true;
    private JPanel leftPanel = new JPanel(new VerticalFlowLayout());

    private JPanel realHardwarePanel = new JPanel(new MigLayout());
    private JPanel miscPanel = new JPanel(new MigLayout()) {
        @Override
        public Dimension getPreferredSize() {
            // want miscPanel and realHardwarePanel to be the same width
            Dimension size = super.getPreferredSize();
            return new Dimension(Math.max(size.width, realHardwarePanel.getPreferredSize().width), size.height);
        }
    };
    /**
     * this flag tells us if we are closing the startup frame in order to proceed with console start or if we are
     * closing the application.
     */
    private boolean isProceeding;
    private JLabel noPortsMessage = new JLabel("No ports found!");

    public StartupFrame() {
        frame = new JFrame("rusEfi console version " + Launcher.CONSOLE_VERSION);
        frame.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
        frame.addWindowListener(new WindowAdapter() {
            @Override
            public void windowClosed(WindowEvent ev) {
                if (!isProceeding) {
                    getConfig().save();
                    System.exit(0);
                }
            }
        });
        setAppIcon(frame);
        SerialPortScanner.INSTANCE.startTimer();
    }

    public static void setAppIcon(JFrame frame) {
        ImageIcon icon = UiUtils.loadIcon(APPICON);
        if (icon != null)
            frame.setIconImage(icon.getImage());
    }

    public void chooseSerialPort() {
        realHardwarePanel.setBorder(new TitledBorder(BorderFactory.createLineBorder(Color.darkGray), "Real stm32"));
        miscPanel.setBorder(new TitledBorder(BorderFactory.createLineBorder(Color.darkGray), "Miscellaneous"));

        if (FileLog.isWindows()) {
            setToolTip(comboPorts, "Use 'Device Manager' icon above to launch Device Manager",
                    "In 'Ports' section look for ",
                    "'STMicroelectronics Virtual COM Port' for USB port",
                    "'USB Serial Port' for TTL port");
        }

        connectPanel.add(comboPorts);
        final JComboBox<String> comboSpeeds = createSpeedCombo();
        comboSpeeds.setToolTipText("For 'STMicroelectronics Virtual COM Port' device any speed setting would work the same");
        connectPanel.add(comboSpeeds);

        final JButton connectButton = new JButton("Connect", new ImageIcon(getClass().getResource("/com/rusefi/connect48.png")));
        //connectButton.setBackground(new Color(RUSEFI_ORANGE)); // custom orange
        setToolTip(connectButton, "Connect to real hardware");
        connectPanel.add(connectButton);
        connectButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                PortHolder.BAUD_RATE = Integer.parseInt((String) comboSpeeds.getSelectedItem());
                String selectedPort = comboPorts.getSelectedItem().toString();
                if (SerialPortScanner.AUTO_SERIAL.equals(selectedPort)) {
                    String autoDetectedPort = PortDetector.autoDetectPort(StartupFrame.this.frame);
                    if (autoDetectedPort == null)
                        return;
                    selectedPort = autoDetectedPort;
                }
                disposeFrameAndProceed();
                new Launcher(selectedPort);
            }
        });

        leftPanel.add(realHardwarePanel);
        leftPanel.add(miscPanel);

        if (FileLog.isWindows()) {
            JPanel topButtons = new JPanel(new FlowLayout(FlowLayout.CENTER, 5, 0));
            topButtons.add(createShowDeviceManagerButton());
            topButtons.add(DriverInstall.createButton());
            realHardwarePanel.add(topButtons, "right, wrap");
        }
        realHardwarePanel.add(connectPanel, "right, wrap");
        realHardwarePanel.add(noPortsMessage, "right, wrap");
        installMessage(noPortsMessage, "Check you cables. Check your drivers. Do you want to start simulator maybe?");

        if (FileLog.isWindows()) {
            realHardwarePanel.add(new HorizontalLine(), "right, wrap");

            ProgramSelector selector = new ProgramSelector(comboPorts);
            realHardwarePanel.add(selector.getControl(), "right, wrap");


//            realHardwarePanel.add(new DfuFlasher(comboPorts).getAutoButton());
//            realHardwarePanel.add(new DfuFlasher(comboPorts).getManualButton());
            // for F7 builds we just build one file at the moment
//            realHardwarePanel.add(new FirmwareFlasher(FirmwareFlasher.IMAGE_FILE, "ST-LINK Program Firmware", "Default firmware version for most users").getButton());
            if (new File(FirmwareFlasher.IMAGE_NO_ASSERTS_FILE).exists()) {
                // 407 build
                FirmwareFlasher firmwareFlasher = new FirmwareFlasher(FirmwareFlasher.IMAGE_NO_ASSERTS_FILE, "ST-LINK Program Firmware/NoAsserts", "Please only use this version if you know that you need this version");
                realHardwarePanel.add(firmwareFlasher.getButton(), "right, wrap");
            }
            realHardwarePanel.add(new EraseChip().getButton(), "right, wrap");
        }

        SerialPortScanner.INSTANCE.listeners.add(new SerialPortScanner.Listener() {
            @Override
            public void onChange() {
                List<String> ports = SerialPortScanner.INSTANCE.getKnownPorts();
                if (!currentlyDisplayedPorts.equals(ports) || isFirstTimeApplyingPorts) {
                    FileLog.MAIN.logLine("Available ports " + ports);
                    isFirstTimeApplyingPorts = false;
                    connectPanel.setVisible(!ports.isEmpty());
                    noPortsMessage.setVisible(ports.isEmpty());
//        panel.add(comboSpeeds); // todo: finish speed selector UI component
//            horizontalLine.setVisible(!ports.isEmpty());

                    applyPortSelectionToUIcontrol(ports);
                    currentlyDisplayedPorts = ports;
                    UiUtils.trueLayout(connectPanel);
                    frame.pack();
                }

            }
        });


        SerialPortScanner.INSTANCE.findAllAvailablePorts();

        final JButton buttonLogViewer = new JButton();
        buttonLogViewer.setText("Start " + LinkManager.LOG_VIEWER);
        buttonLogViewer.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                disposeFrameAndProceed();
                new Launcher(LinkManager.LOG_VIEWER);
            }
        });

        miscPanel.add(buttonLogViewer, "wrap");
        miscPanel.add(new HorizontalLine(), "wrap");

        miscPanel.add(SimulatorHelper.createSimulatorComponent(this));

        JPanel rightPanel = new JPanel(new VerticalFlowLayout());

        ImageIcon logoIcon = UiUtils.loadIcon(LOGO);
        if (logoIcon != null) {
            JLabel logo = new JLabel(logoIcon);
            logo.setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 10));
            URLLabel.addUrlAction(logo, URLLabel.createUri(URI));
            logo.setCursor(new Cursor(Cursor.HAND_CURSOR));
            rightPanel.add(logo);
        }
        rightPanel.add(new URLLabel(LINK_TEXT, URI));
        rightPanel.add(new JLabel("Version " + Launcher.CONSOLE_VERSION));

        JPanel content = new JPanel(new BorderLayout());
        content.add(leftPanel, BorderLayout.WEST);
        content.add(rightPanel, BorderLayout.EAST);
        frame.add(content);
        frame.pack();
        frame.setVisible(true);
        UiUtils.centerWindow(frame);
    }

    private Component createShowDeviceManagerButton() {
        JButton showDeviceManager = new JButton(UiUtils.loadIcon("DeviceManager.png"));
        showDeviceManager.setMargin(new Insets(0, 0, 0, 0));
        showDeviceManager.setToolTipText("Show Device Manager");
        showDeviceManager.addActionListener(event -> {
            try {
                Runtime.getRuntime().exec(ExecHelper.getBatchCommand("devmgmt.msc"));
            } catch (IOException ex) {
                throw new IllegalStateException(ex);
            }
        });
        return showDeviceManager;
    }

    private void installMessage(JComponent component, String s) {
        component.setToolTipText(s);
    }

    public void disposeFrameAndProceed() {
        isProceeding = true;
        frame.dispose();
        SerialPortScanner.INSTANCE.stopTimer();
    }

    private void applyPortSelectionToUIcontrol(List<String> ports) {
        comboPorts.removeAllItems();
        for (final String port : ports)
            comboPorts.addItem(port);
        String defaultPort = getConfig().getRoot().getProperty(Launcher.PORT_KEY);
        comboPorts.setSelectedItem(defaultPort);
    }

    private static JComboBox<String> createSpeedCombo() {
        JComboBox<String> combo = new JComboBox<>();
        String defaultSpeed = getConfig().getRoot().getProperty(Launcher.SPEED_KEY, "115200");
        for (int speed : new int[]{9600, 14400, 19200, 38400, 57600, 115200, 460800, 921600})
            combo.addItem(Integer.toString(speed));
        combo.setSelectedItem(defaultSpeed);
        return combo;
    }
}