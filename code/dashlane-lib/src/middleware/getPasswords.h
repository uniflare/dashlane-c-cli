#pragma once

#include "../Encryption.h"

#include <SQLiteCpp/SQLiteCpp.h>

namespace dlib
{
    // Unicode symbols
    static constexpr char inputSymbolForNumbersCodePoint[] = { 0xF0,0x9F,0x94,0xA2 };
    static constexpr char openLockCodePoint[] = { 0xF0,0x9F,0x94,0x93 };

    struct GetCredential
    {
        std::vector<std::string> filters;
        SSecrets secrets;
        std::string output;
        SQLite::Database db;
    };

    std::vector<STransactionAuthentifiant> decryptPasswordTransactions(
        const std::vector<SRawTransactionBackupEdit>& transactions, 
        const SSecrets& secrets)
    {
        std::vector<STransactionAuthentifiant> credentials;
        for (const auto& transaction : transactions)
        {
            std::unique_ptr<ITransactionBase> pBase = CEncryption::DecryptTransaction(transaction, secrets.localKey);
            if (pBase->type == ETransactionType::Authentifiant)
			{
				credentials.push_back(static_cast<STransactionAuthentifiant&>(*pBase));
            }
        }

        return credentials;
    }

    std::vector<STransactionAuthentifiant> selectCredentials(
        SQLite::Database& db, 
        const SSecrets& secrets, 
        const std::vector<std::string>& filters)
    {
        std::vector<STransactionAuthentifiant> result;

        //SQLite::Statement stmt(db, "SELECT * FROM transactions WHERE login = ? AND action = 'BACKUP_EDIT'");
        //stmt.bind(1, secrets.login);
        //
        //std::vector<BackupEditTransaction> transactions;
        //while (stmt.executeStep())
        //{
        //    BackupEditTransaction transaction;
        //    transaction.identifier = stmt.getColumn("identifier").getString();
        //    transaction.content = stmt.getColumn("content").getString();
        //    transaction.type = stmt.getColumn("type").getString();
        //    transactions.emplace_back(std::move(transaction));
        //}
        //
        //const auto credentialsDecrypted = decryptPasswordTransactions(transactions, secrets);
        return result;
    }

}

/*
export const selectCredentials = async (params: GetCredential): Promise<VaultCredential[]> => {
    const { secrets, filters, db } = params;

    winston.debug(`Retrieving: ${filters && filters.length > 0 ? filters.join(' ') : ''}`);
    const transactions = db
        .prepare(`SELECT * FROM transactions WHERE login = ? AND action = 'BACKUP_EDIT'`)
        .bind(secrets.login)
        .all() as BackupEditTransaction[];

    const credentialsDecrypted = await decryptPasswordTransactions(db, transactions, secrets);

    // transform entries [{_attributes: {key:xx}, _cdata: ww}] into an easier-to-use object
    const beautifiedCredentials = credentialsDecrypted.map(
        (item) =>
            Object.fromEntries(
                item.root.KWAuthentifiant.KWDataItem.map((entry) => [
                    entry._attributes.key[0].toLowerCase() + entry._attributes.key.slice(1), // lowercase the first letter: OtpSecret => otpSecret
                    entry._cdata,
                ])
            ) as unknown as VaultCredential
    );

    let matchedCredentials = beautifiedCredentials;
    if (filters) {
        interface ItemFilter {
            keys: string[];
            value: string;
        }
        const parsedFilters: ItemFilter[] = [];

        filters.forEach((filter) => {
            const [splitFilterKey, ...splitFilterValues] = filter.split('=');

            const filterValue = splitFilterValues.join('=') || splitFilterKey;
            const filterKeys = splitFilterValues.length > 0 ? splitFilterKey.split(',') : ['url', 'title'];

            const canonicalFilterValue = filterValue.toLowerCase();

            parsedFilters.push({
                keys: filterKeys,
                value: canonicalFilterValue,
            });
        });

        matchedCredentials = matchedCredentials?.filter((item) =>
            parsedFilters
                .map((filter) =>
                    filter.keys.map((key) => item[key as keyof VaultCredential]?.toLowerCase().includes(filter.value))
                )
                .flat()
                .some((b) => b)
        );
    }

    return matchedCredentials;
};*/

/*
export const selectCredential = async (params: GetCredential, onlyOtpCredentials = false): Promise<VaultCredential> => {
    let matchedCredentials = await selectCredentials(params);

    if (onlyOtpCredentials) {
        matchedCredentials = matchedCredentials.filter((credential) => credential.otpSecret);
    }

    if (!matchedCredentials || matchedCredentials.length === 0) {
        throw new Error('No credential with this name found');
    } else if (matchedCredentials.length === 1) {
        return matchedCredentials[0];
    }

    return askCredentialChoice({ matchedCredentials, hasFilters: Boolean(params.filters) });
};*/

/*
export const getPassword = async (params: GetCredential): Promise<void> => {
    const clipboard = new Clipboard();
    const selectedCredential = await selectCredential(params);

    switch (params.output || 'clipboard') {
        case 'clipboard':
            clipboard.setText(selectedCredential.password);
            console.log(
                `${openLockCodePoint} Password for "${selectedCredential.title || selectedCredential.url || 'N\\C'}" copied to clipboard!`
            );

            if (selectedCredential.otpSecret) {
                const token = authenticator.generate(selectedCredential.otpSecret);
                const timeRemaining = authenticator.timeRemaining();
                console.log(`${inputSymbolForNumbersCodePoint} OTP code: ${token} \u001B[3m(expires in ${timeRemaining} seconds)\u001B[0m`);
            }
            break;
        case 'password':
            console.log(selectedCredential.password);
            break;
        default:
            throw new Error('Unable to recognize the output mode.');
    }
};*/

/*
export const getOtp = async (params: GetCredential): Promise<void> => {
    const clipboard = new Clipboard();
    const selectedCredential = await selectCredential(params, true);

    // otpSecret can't be null because onlyOtpCredentials is set to true above
    // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
    const token = authenticator.generate(selectedCredential.otpSecret!);
    const timeRemaining = authenticator.timeRemaining();
    switch (params.output || 'clipboard') {
        case 'clipboard':
            clipboard.setText(token);
            console.log(`${inputSymbolForNumbersCodePoint} OTP code: ${token} \u001B[3m(expires in ${timeRemaining} seconds)\u001B[0m`);
            break;
        case 'otp':
            console.log(token);
            break;
        default:
            throw new Error('Unable to recognize the output mode.');
    }
};
*/