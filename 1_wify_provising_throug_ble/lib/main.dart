import 'package:esp_provisioning_wifi/esp_provisioning_bloc.dart';
import 'package:esp_provisioning_wifi/esp_provisioning_event.dart';
import 'package:esp_provisioning_wifi/esp_provisioning_state.dart';
import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return BlocProvider(
      create: (_) => EspProvisioningBloc(),
      child: const MyAppView(),
    );
  }
}

class MyAppView extends StatefulWidget {
  const MyAppView({super.key});

  @override
  State<MyAppView> createState() => _MyAppViewState();
}

class _MyAppViewState extends State<MyAppView> {
  final defaultPadding = 12.0;
  String feedbackMessage = '';

  final prefixController = TextEditingController(text: 'PROV_');
  final proofOfPossessionController = TextEditingController(text: 'abcd1234');
  final ssidController = TextEditingController();
  final passphraseController = TextEditingController();

  // List to hold multiple Wi-Fi network SSID and Password pairs
  List<Map<String, String>> wifiNetworks = [];

  // Function to push feedback to the UI
  void pushFeedback(String msg) {
    setState(() {
      feedbackMessage = '$feedbackMessage\n$msg';
    });
  }

  // Add new Wi-Fi network to the list
  void addNetwork() {
    setState(() {
      wifiNetworks.add({
        'ssid': ssidController.text,
        'password': passphraseController.text,
      });
      ssidController.clear();
      passphraseController.clear();
    });
  }

  @override
  Widget build(BuildContext context) {
    return BlocBuilder<EspProvisioningBloc, EspProvisioningState>(
      builder: (context, state) {
        return MaterialApp(
          home: Scaffold(
            appBar: AppBar(
              title: const Text('ESP BLE Provisioning Example'),
              actions: [
                IconButton(
                  icon: const Icon(Icons.bluetooth),
                  onPressed: () {
                    // Start scanning BLE devices with the given prefix
                    context.read<EspProvisioningBloc>().add(
                        EspProvisioningEventStart(prefixController.text));
                    pushFeedback('Scanning BLE devices');
                  },
                ),
              ],
            ),
            bottomSheet: SafeArea(
              child: Container(
                width: double.infinity,
                color: Colors.black87,
                padding: EdgeInsets.all(defaultPadding),
                child: Text(
                  feedbackMessage,
                  style: TextStyle(
                      fontWeight: FontWeight.bold, color: Colors.green.shade600),
                ),
              ),
            ),
            body: SafeArea(
              child: SingleChildScrollView(
                padding: EdgeInsets.all(defaultPadding),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.stretch,
                  children: [
                    // Device Prefix Input
                    _buildTextField(
                      label: 'Device Prefix',
                      controller: prefixController,
                      hintText: 'Enter device prefix',
                    ),
                    // BLE Devices List
                    _buildSectionHeader('BLE Devices'),
                    _buildDeviceList(state.bluetoothDevices, (device) {
                      context.read<EspProvisioningBloc>().add(
                          EspProvisioningEventBleSelected(device,
                              proofOfPossessionController.text));
                      pushFeedback('Scanning WiFi on $device');
                    }),
                    // Proof of Possession Input
                    _buildTextField(
                      label: 'Proof of possession',
                      controller: proofOfPossessionController,
                      hintText: 'Enter proof of possession string',
                    ),
                    // Wi-Fi Network Inputs (SSID and Password)
                    _buildTextField(
                      label: 'Wi-Fi SSID',
                      controller: ssidController,
                      hintText: 'Enter Wi-Fi SSID',
                    ),
                    _buildTextField(
                      label: 'Wi-Fi Password',
                      controller: passphraseController,
                      hintText: 'Enter Wi-Fi Password',
                      obscureText: true,
                    ),
                    // Add Wi-Fi Network Button
                    ElevatedButton(
                      onPressed: addNetwork,
                      child: const Text('Add Wi-Fi Network'),
                    ),
                    // Wi-Fi Networks List
                    _buildSectionHeader('Wi-Fi Networks'),
                    _buildNetworkList(wifiNetworks, (network) async {
                      // Send Wi-Fi credentials to the selected device
                      context.read<EspProvisioningBloc>().add(
                          EspProvisioningEventWifiSelected(
                              state.bluetoothDevice,
                              proofOfPossessionController.text,
                              network['ssid']!,
                              network['password']!));
                      pushFeedback('Provisioning WiFi ${network['ssid']} on ${state.bluetoothDevice}');
                    }),
                  ],
                ),
              ),
            ),
          ),
        );
      },
    );
  }

  // Helper method to build the input text fields
  Widget _buildTextField({
    required String label,
    required TextEditingController controller,
    required String hintText,
    bool obscureText = false,
  }) {
    return Padding(
      padding: EdgeInsets.symmetric(vertical: defaultPadding),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(label),
          Expanded(
            child: TextField(
              controller: controller,
              decoration: InputDecoration(hintText: hintText),
              obscureText: obscureText,
            ),
          ),
        ],
      ),
    );
  }

  // Helper method to build section headers
  Widget _buildSectionHeader(String title) {
    return Padding(
      padding: EdgeInsets.symmetric(vertical: defaultPadding),
      child: Text(
        title,
        style: TextStyle(fontWeight: FontWeight.bold),
      ),
    );
  }

  // Helper method to build the device list
  Widget _buildDeviceList(List<String> devices, Function(String) onTap) {
    return ListView.builder(
      shrinkWrap: true,
      physics: NeverScrollableScrollPhysics(),
      itemCount: devices.length,
      itemBuilder: (context, index) {
        return ListTile(
          title: Text(
            devices[index],
            style: TextStyle(color: Colors.blue.shade700, fontWeight: FontWeight.bold),
          ),
          onTap: () => onTap(devices[index]),
        );
      },
    );
  }

  // Helper method to build the network list
  Widget _buildNetworkList(List<Map<String, String>> networks, Function(Map<String, String>) onTap) {
    return ListView.builder(
      shrinkWrap: true,
      physics: NeverScrollableScrollPhysics(),
      itemCount: networks.length,
      itemBuilder: (context, index) {
        return ListTile(
          title: Text(
            networks[index]['ssid']!,
            style: TextStyle(color: Colors.green.shade700, fontWeight: FontWeight.bold),
          ),
          subtitle: Text(
            'Password: ${networks[index]['password']}',
            style: TextStyle(color: Colors.grey),
          ),
          onTap: () => onTap(networks[index]),
        );
      },
    );
  }
}
