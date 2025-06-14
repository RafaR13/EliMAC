import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

# Constants
CPU_FREQ_GHZ = 3.2  # From main.c
INPUT_DIR = 'out'
TABLES_DIR = 'tables'
GRAPHS_BASE_DIR = 'graphs'
GRAPH_DIRS = {
    'cycles': f'{GRAPHS_BASE_DIR}/cycles',
    'speed': f'{GRAPHS_BASE_DIR}/speed',
    'speedup': f'{GRAPHS_BASE_DIR}/speedup',
    'variability': f'{GRAPHS_BASE_DIR}/variability'
}

# Create directories
os.makedirs(TABLES_DIR, exist_ok=True)
for dir_path in GRAPH_DIRS.values():
    os.makedirs(dir_path, exist_ok=True)

# Load CSV
try:
    df = pd.read_csv(f'{INPUT_DIR}/elimac_results.csv', sep=';')
except FileNotFoundError:
    print(f"Error: '{INPUT_DIR}/elimac_results.csv' not found. Run 'make run' first.")
    exit(1)

# Data Cleaning
df['CyclesPerByte'] = pd.to_numeric(df['CyclesPerByte'], errors='coerce')
df['TimeUs'] = pd.to_numeric(df['TimeUs'], errors='coerce')
df['MessageLength'] = pd.to_numeric(df['MessageLength'], errors='coerce')
# Calculate Speed (MB/s) = MessageLength (bytes) / TimeUs (µs) * 1e6 (µs/s) / 1e6 (MB)
df['SpeedMBps'] = df['MessageLength'] / (df['TimeUs'] * 1e6) * 1e6
df.dropna(subset=['CyclesPerByte', 'SpeedMBps'], inplace=True)

# Map numeric encodings to labels if needed
encoding_map = {0: 'Naive', 1: 'Compact', 2: 'Custom1', 3: 'Custom2'}
if df['Encoding'].dtype in ['int64', 'float64']:
    df['Encoding'] = df['Encoding'].map(encoding_map).fillna(df['Encoding'])

# Set Seaborn style
sns.set(style="whitegrid", palette="deep", font_scale=1.1)

# Table 1: Mean CyclesPerByte by MessageLength, Encoding, Precompute, Parallel, TagBits, RandomKeys
grouped_cycles = df.groupby(['MessageLength', 'Encoding', 'Precompute', 'Parallel', 'TagBits', 'RandomKeys'])['CyclesPerByte'].mean().unstack('Encoding')
with open(f'{TABLES_DIR}/cycles_table.txt', 'w') as f:
    f.write("Mean CyclesPerByte:\n")
    f.write(grouped_cycles.to_string(float_format="%.2f"))
    valid_encodings = [e for e in ['Compact', 'Variant2', 'Variant3'] if e in grouped_cycles.columns]
    if 'Naive' in grouped_cycles.columns and valid_encodings:
        f.write("\n\nSpeedups (Naive / Other):\n")
        for enc in valid_encodings:
            grouped_cycles[f'Speedup_{enc}'] = grouped_cycles['Naive'] / grouped_cycles[enc]
            f.write(f"\nNaive / {enc}:\n")
            f.write(grouped_cycles[f'Speedup_{enc}'].to_string(float_format="%.2f"))

# Table 2: Mean Speed (MB/s) by MessageLength, Encoding, Precompute, Parallel, TagBits, RandomKeys
grouped_speed = df.groupby(['MessageLength', 'Encoding', 'Precompute', 'Parallel', 'TagBits', 'RandomKeys'])['SpeedMBps'].mean().unstack('Encoding')
with open(f'{TABLES_DIR}/speed_table.txt', 'w') as f:
    f.write("Mean Speed (MB/s):\n")
    f.write(grouped_speed.to_string(float_format="%.2f"))
    valid_encodings = [e for e in ['Compact', 'Variant2', 'Variant3'] if e in grouped_speed.columns]
    if 'Naive' in grouped_speed.columns and valid_encodings:
        f.write("\n\nSpeedups (Other / Naive):\n")
        for enc in valid_encodings:
            grouped_speed[f'Speedup_{enc}'] = grouped_speed[enc] / grouped_speed['Naive']
            f.write(f"\n{enc} / Naive:\n")
            f.write(grouped_speed[f'Speedup_{enc}'].to_string(float_format="%.2f"))

# Graph 1: Cycles/Byte vs. MessageLength for Precompute=0,1, Parallel=0, TagBits=128, RandomKeys=0
for precomp in [0, 1]:
    subset = df[(df['Precompute'] == precomp) & (df['TagBits'] == 128) & (df['RandomKeys'] == 0) & (df['Parallel'] == 0)]
    if subset.empty:
        print(f"Warning: No data for Precompute={precomp}, TagBits=128, RandomKeys=0, Parallel=0")
        continue
    plt.figure(figsize=(10, 6))
    sns.lineplot(data=subset, x='MessageLength', y='CyclesPerByte', hue='Encoding', style='Encoding', markers=True)
    plt.xscale('log')
    plt.title(f'Cycles/Byte vs. Message Length (Precompute={precomp}, Parallel=0, 128-bit Tag, Fixed Keys)')
    plt.xlabel('Message Length (Bytes, Log Scale)')
    plt.ylabel('Cycles/Byte')
    plt.savefig(f'{GRAPH_DIRS["cycles"]}/cycles_precomp_{precomp}_parallel_0.png', dpi=300)
    plt.close()

# Graph 2: Speed (MB/s) vs. MessageLength for Precompute=0,1, Parallel=0, TagBits=128, RandomKeys=0
for precomp in [0, 1]:
    subset = df[(df['Precompute'] == precomp) & (df['TagBits'] == 128) & (df['RandomKeys'] == 0) & (df['Parallel'] == 0)]
    if subset.empty:
        print(f"Warning: No data for Precompute={precomp}, TagBits=128, RandomKeys=0, Parallel=0")
        continue
    plt.figure(figsize=(10, 6))
    sns.lineplot(data=subset, x='MessageLength', y='SpeedMBps', hue='Encoding', style='Encoding', markers=True)
    plt.xscale('log')
    plt.title(f'Speed (MB/s) vs. Message Length (Precompute={precomp}, Parallel=0, 128-bit Tag, Fixed Keys)')
    plt.xlabel('Message Length (Bytes, Log Scale)')
    plt.ylabel('Speed (MB/s)')
    plt.savefig(f'{GRAPH_DIRS["speed"]}/speed_precomp_{precomp}_parallel_0.png', dpi=300)
    plt.close()

# Graph 3: Cycles/Byte vs. MessageLength for Parallel=0,1, Precompute=1, TagBits=128, RandomKeys=0
for parallel in [0, 1]:
    subset = df[(df['Parallel'] == parallel) & (df['Precompute'] == 1) & (df['TagBits'] == 128) & (df['RandomKeys'] == 0)]
    if subset.empty:
        print(f"Warning: No data for Parallel={parallel}, Precompute=1, TagBits=128, RandomKeys=0")
        continue
    plt.figure(figsize=(10, 6))
    sns.lineplot(data=subset, x='MessageLength', y='CyclesPerByte', hue='Encoding', style='Encoding', markers=True)
    plt.xscale('log')
    plt.title(f'Cycles/Byte vs. Message Length (Parallel={parallel}, Precompute=1, 128-bit Tag, Fixed Keys)')
    plt.xlabel('Message Length (Bytes, Log Scale)')
    plt.ylabel('Cycles/Byte')
    plt.savefig(f'{GRAPH_DIRS["cycles"]}/cycles_parallel_{parallel}_precomp_1.png', dpi=300)
    plt.close()

# Graph 4: Speedup (Naive/Other) vs. MessageLength for Precompute=0,1, Parallel=0, TagBits=128, RandomKeys=0
subset = df[(df['TagBits'] == 128) & (df['RandomKeys'] == 0) & (df['Parallel'] == 0)]
if not subset.empty:
    pivot = subset.pivot_table(index=['MessageLength', 'Precompute'], columns='Encoding', values='CyclesPerByte')
    valid_encodings = [e for e in ['Compact', 'Variant2', 'Variant3'] if e in pivot.columns]
    if 'Naive' in pivot.columns and valid_encodings:
        plt.figure(figsize=(10, 6))
        for enc in valid_encodings:
            pivot[f'Speedup_{enc}'] = pivot['Naive'] / pivot[enc]
            temp = pivot.reset_index()[['MessageLength', 'Precompute', f'Speedup_{enc}']].copy()
            temp['Encoding'] = enc
            temp.rename(columns={f'Speedup_{enc}': 'Speedup'}, inplace=True)
            sns.lineplot(data=temp, x='MessageLength', y='Speedup', hue='Encoding', style='Precompute', markers=True)
        plt.xscale('log')
        plt.title('Speedup (Naive/Other) vs. Message Length (Parallel=0, 128-bit Tag, Fixed Keys)')
        plt.xlabel('Message Length (Bytes, Log Scale)')
        plt.ylabel('Speedup (Naive/Other)')
        plt.savefig(f'{GRAPH_DIRS["speedup"]}/speedup_encoding_parallel_0.png', dpi=300)
        plt.close()

# Graph 5: Boxplot of Cycles/Byte by TagBits for Precompute=1, Parallel=0, RandomKeys=0
subset = df[(df['Precompute'] == 1) & (df['Parallel'] == 0) & (df['RandomKeys'] == 0)]
if not subset.empty:
    plt.figure(figsize=(10, 6))
    sns.boxplot(data=subset, x='TagBits', y='CyclesPerByte', hue='Encoding')
    plt.title('Cycles/Byte by Tag Size (Precompute=1, Parallel=0, Fixed Keys)')
    plt.xlabel('Tag Size (Bits)')
    plt.ylabel('Cycles/Byte')
    plt.savefig(f'{GRAPH_DIRS["variability"]}/cycles_by_tagbits_precomp_1.png', dpi=300)
    plt.close()

# Graph 6: Boxplot of Cycles/Byte by RandomKeys for Precompute=1, Parallel=0, TagBits=128
subset = df[(df['Precompute'] == 1) & (df['Parallel'] == 0) & (df['TagBits'] == 128)]
if not subset.empty:
    plt.figure(figsize=(10, 6))
    sns.boxplot(data=subset, x='RandomKeys', y='CyclesPerByte', hue='Encoding')
    plt.title('Cycles/Byte by Key Type (Precompute=1, Parallel=0, 128-bit Tag)')
    plt.xlabel('Random Keys (0=Fixed, 1=Random)')
    plt.ylabel('Cycles/Byte')
    plt.savefig(f'{GRAPH_DIRS["variability"]}/cycles_by_randomkeys_tag128.png', dpi=300)
    plt.close()

# Summary Report
with open(f'{TABLES_DIR}/analysis_summary.txt', 'w') as f:
    f.write("EliMAC Performance Analysis Summary\n")
    f.write("==================================\n\n")
    f.write(f"Data Source: {INPUT_DIR}/elimac_results.csv\n")
    f.write(f"CPU Frequency: {CPU_FREQ_GHZ} GHz\n\n")
    
    # Best Configuration
    best = df.loc[df['CyclesPerByte'].idxmin()]
    f.write("Best Configuration (Lowest Cycles/Byte):\n")
    f.write(f"- Cycles/Byte: {best['CyclesPerByte']:.2f}\n")
    f.write(f"- Message Length: {best['MessageLength']}\n")
    f.write(f"- Encoding: {best['Encoding']}\n")
    f.write(f"- Precompute: {best['Precompute']}\n")
    f.write(f"- Parallel: {best['Parallel']}\n")
    f.write(f"- TagBits: {best['TagBits']}\n")
    f.write(f"- RandomKeys: {best['RandomKeys']}\n\n")
    
    # Key Observations
    f.write("Key Observations:\n")
    valid_encodings = [e for e in ['Compact', 'Variant2', 'Variant3'] if e in grouped_cycles.columns]
    if 'Naive' in grouped_cycles.columns and valid_encodings:
        for enc in valid_encodings:
            mean_speedup = (grouped_cycles['Naive'] / grouped_cycles[enc]).mean()
            f.write(f"- Average Speedup (Naive/{enc}): {mean_speedup:.2f}x\n")
    precomp_diff = grouped_cycles.xs(1, level='Precompute').mean() / grouped_cycles.xs(0, level='Precompute').mean()
    f.write(f"- Precompute=1 vs. Precompute=0 (Avg Cycles/Byte Ratio): {precomp_diff.mean():.2f}x\n")
    parallel_diff = grouped_cycles.xs(1, level='Parallel').mean() / grouped_cycles.xs(0, level='Parallel').mean()
    f.write(f"- Parallel=1 vs. Parallel=0 (Avg Cycles/Byte Ratio): {parallel_diff.mean():.2f}x\n")
    f.write("\nRecommendations:\n")
    f.write("- Optimize for Compact/Variant2/Variant3 if speedup > 1.\n")
    f.write("- Use Precompute for large messages if ratio < 1.\n")
    f.write("- Evaluate Parallel=1 overhead for small messages.\n")
    f.write("- Check AES-NI efficiency if Cycles/Byte > 0.46.\n")

print(f"Analysis complete. Check '{TABLES_DIR}/' for tables and '{GRAPHS_BASE_DIR}/' for graphs.")