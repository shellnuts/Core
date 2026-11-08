#!/usr/bin/env python3
"""Plot CSV telemetry streamed by the firmware (or saved to a file).

Live from the board (sends 's' to start the PID simulation):
    python3 tools/plot_telemetry.py --port /dev/ttyUSB0 --command s
From a saved log:
    python3 tools/plot_telemetry.py --file run.csv

Requires: pip install pyserial matplotlib
Expected format: a header line (e.g. "t,setpoint,y,u"), numeric rows,
and a line starting with "# done" to end the capture. Other lines are ignored.
The first column is used as the x axis.
"""
import argparse
import csv
import sys


def read_lines_serial(port, baud, command, timeout_s):
    import serial  # pyserial

    with serial.Serial(port, baud, timeout=timeout_s) as ser:
        ser.reset_input_buffer()
        if command:
            ser.write(command.encode())
        while True:
            raw = ser.readline()
            if not raw:
                print("timeout waiting for data", file=sys.stderr)
                return
            yield raw.decode(errors="replace").strip()


def read_lines_file(path):
    with open(path, newline="") as f:
        for line in f:
            yield line.strip()


def parse(lines, save_path=None):
    header, rows = None, []
    out = open(save_path, "w", newline="") if save_path else None
    writer = csv.writer(out) if out else None
    for line in lines:
        if line.startswith("# done"):
            break
        fields = line.split(",")
        if header is None:
            if len(fields) >= 2 and all(f and not _is_number(f) for f in fields):
                header = fields
                if writer:
                    writer.writerow(header)
            continue
        if len(fields) != len(header) or not all(_is_number(f) for f in fields):
            continue  # skip shell chatter / partial lines
        row = [float(f) for f in fields]
        rows.append(row)
        if writer:
            writer.writerow(row)
    if out:
        out.close()
    return header, rows


def _is_number(s):
    try:
        float(s)
        return True
    except ValueError:
        return False


def plot(header, rows, image_path=None):
    import matplotlib

    if image_path:
        matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    cols = list(zip(*rows))
    t = cols[0]
    fig, ax = plt.subplots(figsize=(9, 5))
    for name, values in zip(header[1:], cols[1:]):
        ax.plot(t, values, label=name)
    ax.set_xlabel(header[0])
    ax.grid(True, alpha=0.3)
    ax.legend()
    fig.tight_layout()
    if image_path:
        fig.savefig(image_path, dpi=120)
        print(f"saved {image_path}")
    else:
        plt.show()


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    src = ap.add_mutually_exclusive_group(required=True)
    src.add_argument("--port", help="serial port, e.g. /dev/ttyUSB0 or COM5")
    src.add_argument("--file", help="read a saved CSV log instead")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--command", default="", help="text to send before capturing (e.g. s)")
    ap.add_argument("--timeout", type=float, default=5.0, help="serial read timeout [s]")
    ap.add_argument("--save", help="also write the captured data to this CSV")
    ap.add_argument("--image", help="save the plot to this PNG instead of showing it")
    args = ap.parse_args()

    if args.port:
        lines = read_lines_serial(args.port, args.baud, args.command, args.timeout)
    else:
        lines = read_lines_file(args.file)

    header, rows = parse(lines, args.save)
    if not rows:
        sys.exit("no data captured")
    print(f"{len(rows)} samples, columns: {', '.join(header)}")
    plot(header, rows, args.image)


if __name__ == "__main__":
    main()
