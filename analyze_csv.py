import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns  # For better aesthetics

# Load CSV
df = pd.read_csv('out/elimac_results.csv', sep=';')

# Ensure CyclesPerByte is numeric
df['CyclesPerByte'] = pd.to_numeric(df['CyclesPerByte'], errors='coerce')

# Table: Save to file to avoid truncation
grouped = df.groupby(['MessageLength', 'Encoding', 'Precompute', 'Parallel', 'TagBits', 'RandomKeys'])['CyclesPerByte'].mean().unstack('Encoding')
with open('out/results_table.txt', 'w') as f:
    f.write("Mean CyclesPerByte:\n")
    f.write(grouped.to_string(float_format="%.2f"))
    if 'Naive' in grouped.columns and 'Compact' in grouped.columns:
        f.write("\n\nSpeedup (Naive / Compact):\n")
        grouped['Speedup'] = grouped['Naive'] / grouped['Compact']
        f.write(grouped['Speedup'].to_string(float_format="%.2f"))

# Graph: CyclesPerByte for Precompute=0,1, Parallel=0, TagBits=128, RandomKeys=0
sns.set(style="whitegrid")  # Clean style
for precomp in [0, 1]:
    subset = df[(df['Precompute'] == precomp) & (df['TagBits'] == 128) & (df['RandomKeys'] == 0) & (df['Parallel'] == 0)]
    if subset.empty:
        print(f"No data for Precompute={precomp}, TagBits=128, RandomKeys=0, Parallel=0")
        continue
    plt.figure(figsize=(8, 6))
    sns.barplot(data=subset, x='MessageLength', y='CyclesPerByte', hue='Encoding', palette='deep')
    plt.title(f'EliMAC Cycles/Byte (Precompute={precomp}, Parallel=0, 128-bit Tag)')
    plt.ylabel('Cycles/Byte')
    plt.xlabel('Message Length (Bytes)')
    plt.legend(title='Encoding')
    plt.tight_layout()
    plt.savefig(f'out/plot_precomp_{precomp}.png', dpi=300)  # High-res PNG
    plt.show()