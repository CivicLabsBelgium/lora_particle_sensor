const ttn = require('ttn')
const fs = require('fs')
const { exec } = require('child_process')

const log = (logMessage) => {
    process.stdout.write(logMessage + '\n')
}

if (process.env.appID === undefined || process.env.accessKey === undefined || process.env.deviceId === undefined) {
  log('Provide \'appID\' & \'accessKey\' with the environment')
  process.exit(1)
}

const appID = process.env.appID
const accessKey = process.env.accessKey
const deviceId = process.env.deviceId

const globalPath = __dirname + '/src/global.h'

const string2ByteArray = (string) => {
    const byteArray = []
    for (var i = 0; i < string.length; i++) {
        byteArray.push(`0x${string[i].toUpperCase()}${string[i + 1].toUpperCase()}`)
        i++
    }
    return byteArray.join(', ')
}

async function main() {
    const application = await ttn.application(appID, accessKey)

    const app = await application.get()
    console.log(app)

    const euis = await application.getEUIs()

    const device = await application.device(deviceId)
    console.log(device)

    const devAddr = `static const u4_t DEVADDR = 0x${device.devAddr.toUpperCase()};\n`
    const appSKey = `static const u1_t ARTKEY[16] = { ${string2ByteArray(device.appSKey)} };\n`
    const nwkSKey = `static const u1_t DEVKEY[16] = { ${string2ByteArray(device.nwkSKey)} };\n`
    const devEUI = `static const u1_t DEVEUI[8]  = { ${string2ByteArray(device.devEui)} };\n`
    const appvEUI = `static const u1_t APPEUI[8]  = { ${string2ByteArray(device.appEui)} };\n`

    fs.writeFile(globalPath, `${devEUI}\n${appvEUI}\n${appSKey}\n${nwkSKey}\n${devAddr}`, { flag: 'w' }, function (err) {
      if (err) throw err
      console.log("It's saved!")

      exec('platformio run -t upload', (error, stdout, stderr) => {
          if (error) {
              console.error(`exec error: ${error}`);
              return;
          }
          console.log(`stdout: ${stdout}`);
          console.log(`stderr: ${stderr}`);
      })
    });
}

main().catch(function (err) {
  console.error(err)
  process.exit(1)
})
