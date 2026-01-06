import React from "react";

// ArcDisplay: shows an arc with a knob and numeric indicators
// Props: value (number), min (number), max (number), humidity (0-100), currentTemp (number), onChange (function)
export default function ArcDisplay({
  value = 23,
  min = 15,
  max = 35,
  humidity = 41,
  currentTemp = 23,
  onChange,
}) {
  const formatOneDecimal = (val) => (
    Number.isFinite(val) ? val.toFixed(1) : "--"
  );
  const clampValue = (val) => Math.max(min, Math.min(max, val));
  const clamped = clampValue(value);
  const handleChange = (e) => {
    const v = clampValue(Number(e.target.value));
    if (onChange) onChange(v);
  };

  return (
    <div className="w-full flex flex-col items-center space-y-10">
      {/* set temperature */}
      <div className="flex flex-col items-center pointer-events-none">
        <div className="text-xs">set to</div>
        <div className="flex gap-0">
          <div className="text-7xl font-bold text-white">{clamped}</div>
          <div className="text-2xl text-white">°</div>
        </div>
      </div>

      {/* slider control (replaces arc). Uses native range input for accessibility. */}
      <div className="px-6 flex justify-center w-full">
        <input
          type="range"
          min={min}
          max={max}
          step={1}
          value={clamped}
          onChange={handleChange}
          aria-label="Temperature"
          className="w-80 h-4 rounded-full cursor-pointer bg-gray-300 accent-gray-400"
        />
      </div>

      {/* humidity and current temperature below slider */}
      <div className="flex flex-col items-center text-sm">
        <span>Humidity: {Math.round(humidity)}%</span>
        <span>Current Temp: {formatOneDecimal(currentTemp)}°</span>
      </div>
    </div>
  );
}
