import React, { useEffect, useState } from "react";

export default function WifiModal({ open, onClose, onConnect }) {
  const [ssid, setSsid] = useState("");
  const [password, setPassword] = useState("");
  const [error, setError] = useState("");

  useEffect(() => {
    if (!open) return;
    const onKey = (e) => {
      if (e.key === "Escape") onClose();
    };
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [open, onClose]);

  // (intentionally no synchronous state resets in effects to avoid cascading renders)

  if (!open) return null;

  const handleSubmit = (e) => {
    e.preventDefault();
    if (!ssid) {
      setError("Please enter a network name (SSID)");
      return;
    }
    setError("");
    onConnect?.({ ssid, password });
    onClose();
  };

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center m-5">
      <div
        className="absolute inset-0 bg-black/60"
        onClick={onClose}
        aria-hidden="true"
      />

      <div
        role="dialog"
        aria-modal="true"
        className="relative w-full max-w-md bg-black/90 border border-gray-800 rounded-2xl p-6 z-10"
      >
        <h3 className="text-lg font-semibold mb-4 text-white">
          Wi‑Fi Settings
        </h3>

        <form onSubmit={handleSubmit} className="space-y-4">
          <label className="block">
            <div className="text-sm text-gray-300 mb-1 text-left">
              Network (SSID)
            </div>
            <input
              value={ssid}
              onChange={(e) => setSsid(e.target.value)}
              className="w-full rounded-md bg-white/5 border border-gray-700 px-3 py-2 text-white outline-none"
              placeholder="Your Wi‑Fi network"
            />
          </label>

          <label className="block">
            <div className="text-sm text-gray-300 mb-1 text-left">Password</div>
            <input
              type="password"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              className="w-full rounded-md bg-white/5 border border-gray-700 px-3 py-2 text-white outline-none"
              placeholder="Password (optional)"
            />
          </label>

          {error && <div className="text-sm text-red-400">{error}</div>}

          <div className="flex items-center justify-center gap-3 pt-2">
            <button
              type="button"
              onClick={onClose}
              className="px-4 py-2 rounded-md text-gray-300 "
            >
              Cancel
            </button>
            <button
              type="submit"
              className="px-4 py-2 rounded-md  text-white font-medium"
            >
              Connect
            </button>
          </div>
        </form>
      </div>
    </div>
  );
}
