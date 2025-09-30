import Foundation

extension Data {
    init?(hexString: String) {
        let len = hexString.count
        if len % 2 != 0 { return nil }
        var data = Data(capacity: len/2)
        var index = hexString.startIndex
        while index < hexString.endIndex {
            let nextIndex = hexString.index(index, offsetBy: 2)
            let byteString = hexString[index..<nextIndex]
            if let byte = UInt8(byteString, radix: 16) {
                data.append(byte)
            } else {
                return nil
            }
            index = nextIndex
        }
        self = data
    }
}


